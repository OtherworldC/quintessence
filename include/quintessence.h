#ifndef QUINTESSENCE_H_
#define QUINTESSENCE_H_

#ifndef PORT
#define PORT 8080
#endif

#ifndef BUFSIZE
#define BUFSIZE 16384
#endif

#include "internal/structs.h"
#include <stdbool.h>

qu_config_t new_server(char basepath[]);
int run_qu_server(qu_config_t config);
void register_handler(qu_config_t config, char* method, char* uri, handler_function_t handler);

#endif