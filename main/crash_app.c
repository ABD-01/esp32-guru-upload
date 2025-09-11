#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "app_tasks.h"

const static char TAG[] __attribute__((unused)) = "crash_app";

static int g_xyz;
static int foo(uint32_t val);
static void bar(int val);
static int baz(int val);

void CrashTask(void *pvParameters)
{
    g_xyz = esp_cpu_get_cycle_count() + 5928;
    if (!g_xyz) {
        g_xyz = 42;
    }

    while(1)
    {
        uint32_t r = get_rand((uint32_t *)&g_xyz) % 190 + 1;
        foo(r);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static int foo(uint32_t val)
{
    ESP_APP_LOGI("foo called with %u", val);
    bar(val);
    return val;
}

static void bar(int val)
{
    ESP_APP_LOGI("bar called with %d", val);
    int i = val + 32;
    int j = baz(i);
    (void)j;
    return;
}

static int baz(int val)
{
    ESP_APP_LOGI("baz called with %d", val);
    if (val < 63 || val > 100) {
        return 78;
    } else {
        ESP_APP_LOGE("Crashing now! val=%d", val);
        volatile uint32_t *bat_ptr = (uint32_t *)0xDEADBEEF;
        *bat_ptr                   = 0x12345678;
        return 9;
    }
}