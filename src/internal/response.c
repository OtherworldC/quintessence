#include "internal/structs.h"
#include "internal/response.h"
#include "quintessence.h"
#include <stdio.h>
#include <string.h>

#define STAT_STR_MAC(code, str) {\
                    case code:{\
                        strcpy(str_buf, str);\
                        break;\
                    }}

void get_status_string(char* str_buf, int status) {
    switch (status) {
        STAT_STR_MAC(200, "OK");
        STAT_STR_MAC(404, "Not Found")
        STAT_STR_MAC(500, "Internal Server Error");
    }
}

void fallback_404_func(request_ctx_t* ctx) {
    ctx->status = 404;
    write_response(ctx, "<html><body><h1>404 Not Found</h1></body></html>", "text/html");
}

char TEMPLATE_STRING[] = "HTTP/1.0 %d %s\r\nServer: Quintessence/1.0\r\nContent-type: %s\r\n\r\n%s";

void create_response(qu_config_t* config, request_ctx_t* ctx) {
    // call handler function
    handler_t item;
    strcpy(item.route.hashkey, (*ctx).method);
    strcat(item.route.hashkey, (*ctx).uri);

    handler_t* handler_func;
    handler_func = (handler_t*) hashmap_get((*config).handlers, &item);

    if (handler_func != NULL) {
        handler_func->func(ctx);
    } else {
        // 404 Not Found
        printf("handler_func was NULL. Func not called.\n");
        fallback_404_func(ctx);
    }
    // return status;
}

void write_response(request_ctx_t* ctx, char content[], char content_type[]) {
    char status_str_buf[128];
    get_status_string(status_str_buf, (*ctx).status);
    sprintf((*ctx).response, TEMPLATE_STRING, (*ctx).status, status_str_buf, content_type, content);
}