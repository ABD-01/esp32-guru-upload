#include "http_service.h"
#include "app_tasks.h"

const static char TAG[] __attribute__((unused)) = "http_service";

esp_http_client_handle_t http_client_init(const char *uri, size_t total_len)
{
    esp_http_client_config_t config = {
        .url = uri,
        .method = HTTP_METHOD_POST,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "application/octet-stream");

    if (esp_http_client_open(client, total_len) != ESP_OK) {
        esp_http_client_cleanup(client);
        return NULL;
    }
    return client;
}

esp_err_t http_upload_send_chunk(esp_http_client_handle_t client, const uint8_t *data, size_t len)
{
    int wlen = esp_http_client_write(client, (const char *)data, len);
    if (wlen < 0) {
        return ESP_FAIL;
    }
    return ESP_OK;
}

int http_client_close(esp_http_client_handle_t client)
{
    if (!client) return -1;
    int content_length = esp_http_client_fetch_headers(client);
    ESP_APP_LOGI("Response content length: %d", content_length);
    int status = esp_http_client_get_status_code(client);
    ESP_APP_LOGI("Response status = %d", status);

    char output_buffer[64] = {0};
    if (content_length > 0) {
        int data_read = esp_http_client_read_response(client, output_buffer, sizeof(output_buffer) - 1);
        if (data_read >= 0) {
            ESP_APP_LOGI("Response body:\r\n    %s\r\n", output_buffer);
        } else {
            ESP_APP_LOGE("Failed to read response");
        }
    }

    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    return status;
}


esp_err_t http_post_chunks(const char *uri, const uint8_t *data, size_t total_len)
{
    // Ref: https://github.com/espressif/esp-idf/blob/f38b8fec92a0e389b733cb4fbf49be86e5144333/examples/protocols/esp_http_client/main/esp_http_client_example.c#L763-L792
    esp_err_t err = ESP_OK;
    esp_http_client_config_t config = {
        .url = uri,
        .method = HTTP_METHOD_POST,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_header(client, "Content-Type", "application/octet-stream");

    err = esp_http_client_open(client, total_len);
    if (err != ESP_OK) {
        ESP_APP_LOGE("Failed to open connection: %s", esp_err_to_name(err));
        return err;
    }

    size_t sent = 0;
    while (sent < total_len) {
        size_t chunk_len = CHUNK_SIZE;
        if (total_len - sent < CHUNK_SIZE) {
            chunk_len = total_len - sent;
        }
        int wlen = esp_http_client_write(client, (const char *)(data + sent), chunk_len);
        if (wlen < 0) {
            ESP_APP_LOGE("Write failed");
            err = ESP_FAIL;
            break;
        }
        sent += wlen;
        ESP_APP_LOGD("Sent %d/%d bytes", sent, total_len);
    }

    int content_length = esp_http_client_fetch_headers(client);
    ESP_APP_LOGI("Response content length: %d", content_length);

    int status = esp_http_client_get_status_code(client);
    ESP_APP_LOGI("POST finished, status = %d", status);
    if (status != HttpStatus_Ok) {
        ESP_APP_LOGE("HTTP POST request failed with status %d", status);
        err = ESP_FAIL;
    }

    esp_http_client_close(client);
    esp_http_client_cleanup(client);

    return err;
}