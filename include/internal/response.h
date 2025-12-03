#ifndef QUINTESSENCE_RESPONSE_H_
#define QUINTESSENCE_RESPONSE_H_

#include "internal/structs.h"
int create_response(qu_config_t* config, request_ctx_t* ctx);
void write_response(request_ctx_t* ctx, char content[], char content_type[]);
int get_mime_type(const char* path, char* out_buf, size_t buflen);
int serve_static_file(qu_config_t* config, request_ctx_t* ctx);
int serve_large_file(qu_config_t* config, request_ctx_t* ctx);

#endif