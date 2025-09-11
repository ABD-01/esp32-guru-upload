#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "app_tasks.h"
#include "http_service.h"

#include "esp_core_dump.h"
#include "esp_partition.h"
#include "esp_http_client.h"

#define UPLOAD_SERVER_HOST  "10.0.2.2"
#define UPLOAD_SERVER_PORT  8080
#define UPLOAD_PATH         "/upload"

const static char TAG[] __attribute__((unused)) = "upload_task";

static uint8_t g_buf[CHUNK_SIZE];

void UploadCoredumpTask(void *pvParameters)
{
    const esp_partition_t *core_part = NULL;
    uint32_t core_size = 0;
    core_part = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_COREDUMP, NULL);
    if(core_part == NULL || core_part->size < sizeof(uint32_t)) {
        ESP_APP_LOGE("No coredump partition found");
        vTaskDelete(NULL);
        return;
    }
    esp_err_t err = esp_partition_read(core_part, 0, &core_size, sizeof(uint32_t));
    if (err != ESP_OK) {
        ESP_APP_LOGE("Failed to read core dump data size (%d)!", err);
        vTaskDelete(NULL);
        return;
    }

    if (core_size == 0 || core_size > core_part->size - sizeof(uint32_t) || core_size == 0xFFFFFFFF) {
        ESP_APP_LOGE("Invalid coredump size %d", core_size);
        vTaskDelete(NULL);
        return;
    }

    ESP_APP_LOGI("Coredump size: %d bytes", core_size);
    ESP_APP_LOGI("Reading coredump from partition '%s'", core_part->label);

    // HTTP part [start]
    char url[128];
    snprintf(url, sizeof(url), "http://%s:%d%s", UPLOAD_SERVER_HOST, (uint16_t)UPLOAD_SERVER_PORT, UPLOAD_PATH);
    esp_http_client_handle_t client = http_client_init(url, core_size);
    if (client == NULL)
    {
        ESP_APP_LOGE("Failed to open connection");
        vTaskDelete(NULL);
        return;
    }
    if (http_upload_send_chunk(client, (const uint8_t *)&core_size, sizeof(uint32_t)) != ESP_OK)
    {
        ESP_APP_LOGE("Failed to send size");
        vTaskDelete(NULL);
        return;
    }
    // HTTP part [end]

    uint8_t *buf = g_buf;
    size_t offset = 0;
    while (offset < core_size) {
        size_t chunk = (core_size - offset) > CHUNK_SIZE ? CHUNK_SIZE : (core_size - offset);
        err = esp_partition_read(core_part, offset + sizeof(uint32_t), buf, chunk);
        if (err != ESP_OK) {
            ESP_APP_LOGE("Failed to read coredump data (%d)!", err);
            vTaskDelete(NULL);
            return;
        }

        // HTTP part [start]
        if(http_upload_send_chunk(client, buf, chunk) != ESP_OK)
        {
            ESP_APP_LOGE("Failed to send chunk");
            vTaskDelete(NULL);
            return;
        }
        // HTTP part [end]

        offset += chunk;
        // vTaskDelay(pdMS_TO_TICKS(1));
        ESP_APP_LOGI("Uploaded %d/%d bytes", offset, core_size);
    }

    // HTTP part [start]
    int status = http_client_close(client);
    ESP_APP_LOGI("Upload finished, status = %d", status);
    // HTTP part [end]

    vTaskDelete(NULL);

}

