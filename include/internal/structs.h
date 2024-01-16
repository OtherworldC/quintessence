#ifndef QUINTESSENCE_STRUCTS_H_
#define QUINTESSENCE_STRUCTS_H_
#include <arpa/inet.h>
#include "internal/router/hashmap.h"
#include "internal/router/thpool.h"


typedef struct QuRequestContext {
    int error;
    char uri[1024];
    char method[64];
    char headers[1024][1024];
    char request_ip[1024];
    char version[64];
    char response[16384];
    int status;
} request_ctx_t;

typedef struct QuRouteInfo {
    char uri[1024];
    char method[1024];
    char hashkey[1024];
} route_info_t;

typedef void (*handler_function_t)(request_ctx_t*);

// map uri+method to handler function w/ hashmap (route field is key, func is val)
typedef struct Handler {
    handler_function_t func;
    route_info_t route;
} handler_t;

typedef struct QuConfig {
    int sockfd;
    threadpool thpool;
    struct sockaddr_in host_addr;
    int host_addrlen;
    // handler_t handlers[1024];
    struct hashmap* handlers;
    char basepath[];
} qu_config_t;

typedef struct RequestThreadArg {
    int newsockfd;
    qu_config_t* config;
} req_arg_t;

#endif