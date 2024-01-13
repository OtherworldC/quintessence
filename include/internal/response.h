#ifndef QUINTESSENCE_RESPONSE_H_
#define QUINTESSENCE_RESPONSE_H_

#include "internal/structs.h"
void create_response(qu_config_t* config, request_ctx_t* ctx);
void write_response(request_ctx_t* ctx, char content[], char content_type[]);

#endif