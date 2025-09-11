#ifndef __HTTP_SERVICE_H
#define __HTTP_SERVICE_H

#include "esp_http_server.h"
#include "esp_http_client.h"

#ifndef CHUNK_SIZE
#define CHUNK_SIZE 1024
#endif

esp_err_t http_post_chunks(const char *, const uint8_t *, size_t);
esp_http_client_handle_t http_client_init(const char *, size_t );
esp_err_t http_upload_send_chunk(esp_http_client_handle_t, const uint8_t *data, size_t len);
int http_client_close(esp_http_client_handle_t);


#endif /* __HTTP_SERVICE_H */