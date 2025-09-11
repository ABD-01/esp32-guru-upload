#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "app_tasks.h"

static uint32_t rand_state;

void RandomTask(void *pvParameters)
{
    rand_state = esp_cpu_get_cycle_count() + 42;

    while (1) {
        uint32_t rand = get_rand(&rand_state);
        int action = rand % 8;

        switch (action) {
        case 0:
            ESP_LOGI("dummy_gpio","Toggling LED Num %lu", rand % 14);
            break;
        case 1:
            ESP_LOGI("dummy_gps", "Found GPS Latitude: %lu, Longitude: %lu",
                   (rand >> 16) & 0xFFFF, rand & 0xFFFF);
            break;
        case 2:
            ESP_LOGI("dummy_sens", "Temperature Sensor Value: %lu", rand % 100);
            break;
        case 3:
            ESP_LOGI("dummy_sens","Pressure Sensor Reading: %lu", rand % 1000);
            break;
        case 4:
            ESP_LOGI("dummy_battery", "Battery Voltage: %lu mV", rand % 5000);
            break;
        case 5:
            ESP_LOGI("dummy_sys", "Random Debug Event ID: %ld", rand & 0xFF);
            break;
        case 6:
            ESP_LOGI("dummy_sys", "System Heartbeat Tick: %lu", rand);
            break;
        case 7:
            ESP_LOGW("dummy_sys", "Noise Data Sample: %ld", rand);
            rand_state = esp_cpu_get_cycle_count() + 42;
            break;
        default:
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
