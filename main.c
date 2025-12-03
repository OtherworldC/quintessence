#include <stdio.h>
#include <stdlib.h>

#include "internal/response.h"
#include "quintessence.h"

void handle_req(request_ctx_t* ctx) {
    printf("Served main.html.\n");
    ctx->status = 200;
    write_response(ctx, "<html><body><h1>Hiiii!</h1></body></html>", "text/html");
}

int main() {
    printf("Hello from Quintessence.\n");
    qu_config_t config;
    new_server(&config, "./serve");
    register_handler(&config, "GET", "/main", handle_req);
    int rval = run_qu_server(&config);
    return rval;
}