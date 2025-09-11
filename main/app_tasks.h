#ifndef __APP_TASKS_H
#define __APP_TASKS_H

#include "esp_log.h"

#define ESP_APP_LOG( level, format, ... )  if (LOG_LOCAL_LEVEL >= level)   { esp_rom_printf((format), esp_log_early_timestamp(), (const char *)TAG, ##__VA_ARGS__); }

#define ESP_APP_LOGE( format, ... )  ESP_APP_LOG(ESP_LOG_ERROR, LOG_FORMAT(E, format), ##__VA_ARGS__)
#define ESP_APP_LOGW( format, ... )  ESP_APP_LOG(ESP_LOG_WARN, LOG_FORMAT(W, format), ##__VA_ARGS__)
#define ESP_APP_LOGI( format, ... )  ESP_APP_LOG(ESP_LOG_INFO, LOG_FORMAT(I, format), ##__VA_ARGS__)
#define ESP_APP_LOGD( format, ... )  ESP_APP_LOG(ESP_LOG_DEBUG, LOG_FORMAT(D, format), ##__VA_ARGS__)
#define ESP_APP_LOGV( format, ... )  ESP_APP_LOG(ESP_LOG_VERBOSE, LOG_FORMAT(V, format), ##__VA_ARGS__)


#define STACK_CRASH         (1024*2)
#define STACK_RANDOM        (1024*4)
#define STACK_UPLOAD        (1024*8)

void CrashTask(void *pvParameters);
void RandomTask(void *pvParameters);
void UploadCoredumpTask(void *pvParameters);

static inline uint32_t get_rand(uint32_t *state)
{
    // xorshift-like
    *state ^= (*state << 13);
    *state ^= (*state >> 17);
    *state ^= (*state << 5);
    return *state;
}


#endif /* __APP_TASKS_H */