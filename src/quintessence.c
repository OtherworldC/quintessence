#include "quintessence.h"
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include "internal/context.h"
#include "internal/response.h"
#include "internal/log/qulog.h"


int handler_compare(const void* a, const void* b, void* hdata) {
    const handler_t* ha = a;
    const handler_t* hb = b;
    return strcmp(ha->route.hashkey, hb->route.hashkey);
}

bool handler_iter(const void* item, void* hdata) {
    const handler_t* handler = item;
    printf("Method: %s Route: %s Function: %p Hashkey: %s", handler->route.method, handler->route.uri, (void*) handler->func, handler->route.hashkey);
    return true;
}

uint64_t handler_hash(const void* item, uint64_t seed0, uint64_t seed1) {
    const handler_t* handler = item;
    return hashmap_sip(handler->route.hashkey, strlen(handler->route.hashkey), seed0, seed1);
}

qu_config_t new_server(char basepath[]) {
    qu_config_t config;
    strcpy(config.basepath, basepath);
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int option = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    if (sockfd == -1) {
        perror("quintessence (socket)");
        exit(-1);
    }
    printf("socket created\n");

    struct sockaddr_in host_addr;
    int host_addrlen = sizeof(host_addr);
    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(PORT);
    host_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    config.sockfd = sockfd;
    config.host_addr = host_addr;
    config.host_addrlen = host_addrlen;

    config.handlers = hashmap_new(sizeof(handler_t), 0, 0, 0, handler_hash, handler_compare, NULL, NULL);
    return config;
}

void shutdown_server(qu_config_t config) {
    hashmap_free(config.handlers);
}

int run_qu_server(qu_config_t config) {
    struct sockaddr_in client_addr;
    int client_addrlen = sizeof(client_addr);

    // bind socket to address
    if (bind(config.sockfd, (struct sockaddr *)&config.host_addr, config.host_addrlen) != 0) {
        perror("quintessence (bind)");
        return -1;
    }
    printf("socket bound to address\n");

    if (listen(config.sockfd, SOMAXCONN) != 0) {
        perror("quintessence (listen)");
        return -1;
    }
    printf("listening for connections...\n");

    hashmap_scan(config.handlers, handler_iter, NULL);
    printf("\n");

    for (;;) {
        // accept incoming connections
        int newsockfd = accept(config.sockfd, (struct sockaddr *)&config.host_addr, (socklen_t *)&config.host_addrlen);

        struct timespec starttime;
        timespec_get(&starttime, TIME_UTC);

        if (newsockfd < 0) {
            perror("quintessence (accept)");
            return 1;
        }

        request_ctx_t ctx;
        extract_context(&ctx, newsockfd, &client_addr, &client_addrlen);

        if (ctx.error != 0) {
            printf("[QUINTESSENCE] \033[1;37;41m Unable to handle request. See above.\033[0m\n");
            continue;
        }

        char reqdata[1024];
        sprintf(reqdata, "%s | %s | %s\n", ctx.request_ip, ctx.method, ctx.uri);
        // write to socket
        create_response(&config, &ctx);
        int valwrite = write(newsockfd, ctx.response, strlen(ctx.response));
        if (valwrite < 0) {
            perror("quintessence (write)");
            continue;
        }

        // sleep(1);
        struct timespec now;
        timespec_get(&now, TIME_UTC);
        int diff;
        diff = gmtime(&now.tv_sec)->tm_sec - gmtime(&starttime.tv_sec)->tm_sec;
        char ftime[80];
        sprintf(ftime, "%d.%09lds", diff, now.tv_nsec-starttime.tv_nsec);

        log_request(ctx.status, reqdata, ftime);
        close(newsockfd);
    }
    shutdown_server(config);
    return 0;
}

void register_handler(qu_config_t config, char* method, char* uri, handler_function_t handler) {
    char hashkey[1024];
    strcpy(hashkey, method);
    strcat(hashkey, uri);

    handler_t ht;
    ht.func = handler;
    strcpy(ht.route.hashkey, hashkey);
    strcpy(ht.route.method, method);
    strcpy(ht.route.uri, uri);
    
    hashmap_set(config.handlers, &ht);
}
