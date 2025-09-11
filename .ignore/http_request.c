/* HTTP GET Example using plain POSIX sockets

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "sdkconfig.h"

#include "esp_http_client.h"
#include "app_tasks.h"

const static char TAG[] __attribute__((unused)) = "http_request";

/* Constants that aren't configurable in menuconfig */
// #define WEB_SERVER "example.com"
// #define WEB_PORT "80"
// #define WEB_PATH "/"

#define WEB_SERVER "10.0.2.2"
#define WEB_PORT "8080"
// #define WEB_PATH "/hello.txt"
#define WEB_PATH "/upload"


static const char *REQUEST = "GET " WEB_PATH " HTTP/1.0\r\n"
    "Host: "WEB_SERVER":"WEB_PORT"\r\n"
    "User-Agent: esp-idf/1.0 esp32\r\n"
    "\r\n";

static void http_get_task(void *pvParameters)
{
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s, r;
    char recv_buf[64];

    while(1) {
        int err = getaddrinfo(WEB_SERVER, WEB_PORT, &hints, &res);

        if(err != 0 || res == NULL) {
            ESP_APP_LOGE("DNS lookup failed err=%d res=%p", err, res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        /* Code to print the resolved IP.

           Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_APP_LOGI("DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

        s = socket(res->ai_family, res->ai_socktype, 0);
        if(s < 0) {
            ESP_APP_LOGE("... Failed to allocate socket.");
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_APP_LOGI("... allocated socket");

        if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
            ESP_APP_LOGE("... socket connect failed errno=%d", errno);
            close(s);
            freeaddrinfo(res);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_APP_LOGI("... connected");
        freeaddrinfo(res);

        if (write(s, REQUEST, strlen(REQUEST)) < 0) {
            ESP_APP_LOGE("... socket send failed");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_APP_LOGI("... socket send success");

        struct timeval receiving_timeout;
        receiving_timeout.tv_sec = 5;
        receiving_timeout.tv_usec = 0;
        if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
                sizeof(receiving_timeout)) < 0) {
            ESP_APP_LOGE("... failed to set socket receiving timeout");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_APP_LOGI("... set socket receiving timeout success");

        /* Read HTTP response */
        do {
            bzero(recv_buf, sizeof(recv_buf));
            r = read(s, recv_buf, sizeof(recv_buf)-1);
            for(int i = 0; i < r; i++) {
                putchar(recv_buf[i]);
            }
        } while(r > 0);

        ESP_APP_LOGI("... done reading from socket. Last read return=%d errno=%d.", r, errno);
        close(s);
        for(int countdown = 10; countdown >= 0; countdown--) {
            ESP_APP_LOGI("%d... ", countdown);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        ESP_APP_LOGI("Starting again!");
    }
}

static void http_post_bin_task(void *pv)
{
    vTaskDelay(pdMS_TO_TICKS(3000)); // wait for IP

    const char *host = WEB_SERVER;
    const char *port = WEB_PORT;

    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res = NULL;

    if (getaddrinfo(host, port, &hints, &res) != 0) {
        ESP_APP_LOGE("DNS lookup failed");
        vTaskDelete(NULL);
    }

    int s = socket(res->ai_family, res->ai_socktype, 0);
    if (s < 0) {
        ESP_APP_LOGE("Failed to create socket");
        freeaddrinfo(res);
        vTaskDelete(NULL);
    }

    if (connect(s, res->ai_addr, res->ai_addrlen) != 0) {
        ESP_APP_LOGE("Socket connect failed");
        close(s);
        freeaddrinfo(res);
        vTaskDelete(NULL);
    }
    freeaddrinfo(res);

    // --- Binary payload example ---
    uint8_t payload[8] = {0xde, 0xad, 0xbe, 0xef, 0x01, 0x02, 0x03, 0x04};
    int body_len = sizeof(payload);

    char header[256];
    int hlen = snprintf(header, sizeof(header),
                        "POST %s HTTP/1.1\r\n"
                        "Host: %s\r\n"
                        "Content-Type: application/octet-stream\r\n"
                        "Content-Length: %d\r\n"
                        "\r\n",
                        WEB_PATH, host, body_len);

    // send headers
    send(s, header, hlen, 0);

    // send raw binary
    send(s, payload, body_len, 0);

    // recv response
    char buf[256];
    int rlen = recv(s, buf, sizeof(buf)-1, 0);
    if (rlen > 0) {
        buf[rlen] = 0;
        ESP_APP_LOGI("HTTP POST binary response:\n%s", buf);
    } else {
        ESP_APP_LOGE("No response or recv failed");
    }

    close(s);
    vTaskDelete(NULL);
}

#define CHUNK_SIZE 1024  // send 1 KB at a time

static void http_post_large_task(void *pv)
{
    vTaskDelay(pdMS_TO_TICKS(3000)); // wait for IP

    const char *host = WEB_SERVER;
    const char *port = WEB_PORT;

    struct addrinfo hints = {0}, *res = NULL;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, port, &hints, &res) != 0) {
        ESP_APP_LOGE("DNS lookup failed");
        vTaskDelete(NULL);
    }

    int s = socket(res->ai_family, res->ai_socktype, 0);
    if (s < 0) {
        ESP_APP_LOGE("Failed to create socket");
        freeaddrinfo(res);
        vTaskDelete(NULL);
    }

    if (connect(s, res->ai_addr, res->ai_addrlen) != 0) {
        ESP_APP_LOGE("Socket connect failed");
        close(s);
        freeaddrinfo(res);
        vTaskDelete(NULL);
    }
    freeaddrinfo(res);

    // --- Example: large payload (simulate coredump) ---
    size_t payload_size = 8192;  // 8 KB
    uint8_t *payload = malloc(payload_size);
    if (!payload) {
        ESP_APP_LOGE("malloc failed");
        close(s);
        vTaskDelete(NULL);
    }

    // Fill with dummy data or your coredump
    for (size_t i = 0; i < payload_size; i++) payload[i] = i % 256;

    // Send HTTP headers first
    char header[256];
    int hlen = snprintf(header, sizeof(header),
                        "POST %s HTTP/1.1\r\n"
                        "Host: %s\r\n"
                        "Content-Type: application/octet-stream\r\n"
                        "Content-Length: %u\r\n"
                        "\r\n",
                        WEB_PATH, host, (unsigned)payload_size);
    send(s, header, hlen, 0);

    // Send payload in chunks
    size_t sent = 0;
    while (sent < payload_size) {
        size_t to_send = (payload_size - sent) > CHUNK_SIZE ? CHUNK_SIZE : (payload_size - sent);
        int ret = send(s, payload + sent, to_send, 0);
        if (ret <= 0) {
            ESP_APP_LOGE("Send failed");
            break;
        }
        sent += ret;
    }

    // Receive server response
    char buf[256];
    int rlen = recv(s, buf, sizeof(buf)-1, 0);
    if (rlen > 0) {
        buf[rlen] = 0;
        ESP_APP_LOGI("Response:\n%s", buf);
        ESP_APP_LOGI("Sent %u bytes, server responded with %d bytes", (unsigned)sent, rlen);
    }

    free(payload);
    close(s);
    vTaskDelete(NULL);
}

