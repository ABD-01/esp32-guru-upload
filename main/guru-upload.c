#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_core_dump.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"

#include "app_tasks.h"

const static char TAG[] __attribute__((unused)) = "main_app";

extern void http_post_chunks(const char *, const uint8_t *, size_t);

void app_main(void)
{
    xTaskCreate(&RandomTask, "RandomTask", STACK_RANDOM, NULL, 5, NULL);

    size_t addr = 0, size = 0;
    esp_err_t err = esp_core_dump_image_get(&addr, &size);
    if (err == ESP_OK) {
        ESP_APP_LOGI("Coredump file found at address 0x%08x with size %d bytes", addr, size);
    } else {
        ESP_APP_LOGI("No coredump file found. esp_core_dump_image_get() returned %d", err);
        vTaskDelay(pdMS_TO_TICKS(2*1000));
        xTaskCreate(&CrashTask, "CrashTask", STACK_CRASH, NULL, 5, NULL);
    }

    ESP_ERROR_CHECK( nvs_flash_init() );
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    vTaskDelay(pdMS_TO_TICKS(10*1000));

    xTaskCreate(&UploadCoredumpTask, "UploadCoredumpTask", STACK_UPLOAD, NULL, 5, NULL);
}
