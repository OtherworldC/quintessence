#ifndef QUINTESSENCE_CONTEXT_H_
#define QUINTESSENCE_CONTEXT_H_

#include "internal/structs.h"

void extract_context(request_ctx_t* ctx, int newsockfd, struct sockaddr_in* client_addr, int* client_addrlen);

#endif