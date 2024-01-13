#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "internal/structs.h"
#include "quintessence.h"

void extract_context(request_ctx_t* ctx, int newsockfd, struct sockaddr_in* client_addr, int* client_addrlen) {
    char buffer[BUFSIZE];

    // get client details
    int sockn = getsockname(newsockfd, (struct sockaddr *)client_addr, (socklen_t *)client_addrlen);
    if (sockn < 0) {
        perror("quintessence (getsockname)");
        ctx->error = 1;
        return;
    }

    // read from socket
    int valread = read(newsockfd, buffer, BUFSIZE);
    if (valread < 0) {
        perror("quintessence (read)");
        ctx->error = 1;
        return;
    }

    // Read the request
    // char method[BUFSIZE], uri[BUFSIZE], version[BUFSIZE];
    sscanf(buffer, "%s %s %s", ctx->method, ctx->uri, ctx->version);
    // if (!strcmp(ctx->uri, "/")) {
    //     strcpy(ctx->uri, "/main.html");
    // }

    sprintf(ctx->request_ip, "%s", inet_ntoa((*client_addr).sin_addr));
}