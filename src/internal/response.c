#include "internal/structs.h"
#include "internal/response.h"
#include "quintessence.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <limits.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

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

int create_response(qu_config_t* config, request_ctx_t* ctx) {
    // call handler function
    handler_t item;
    strcpy(item.route.hashkey, (*ctx).method);
    strcat(item.route.hashkey, (*ctx).uri);

    handler_t* handler_func;
    handler_func = (handler_t*) hashmap_get((*config).handlers, &item);

    if (handler_func != NULL) {
        handler_func->func(ctx);
    } else {
        int static_check = serve_static_file(config, ctx);
        if (static_check == 0) {
            return 0;
        } else if (static_check == 2) {
            if (serve_large_file(config, ctx) == 0) {
                return 1;
            }
        }
        // 404 Not Found
        printf("handler_func was NULL. Func not called.\n");
        fallback_404_func(ctx);
    }
    return 0;
}

int get_mime_type(const char* path, char* out_buf, size_t buflen) {
    const char* ext = strrchr(path, '.');
    if (!ext) { strncpy(out_buf, "application/octet-stream", buflen); return 0; }
    ext++; // skip '.'
    if (strcasecmp(ext, "html") == 0 || strcasecmp(ext, "htm") == 0) strncpy(out_buf, "text/html", buflen);
    else if (strcasecmp(ext, "css") == 0) strncpy(out_buf, "text/css", buflen);
    else if (strcasecmp(ext, "js") == 0) strncpy(out_buf, "application/javascript", buflen);
    else if (strcasecmp(ext, "png") == 0) strncpy(out_buf, "image/png", buflen);
    else if (strcasecmp(ext, "jpg") == 0 || strcasecmp(ext, "jpeg") == 0) strncpy(out_buf, "image/jpeg", buflen);
    else if (strcasecmp(ext, "gif") == 0) strncpy(out_buf, "image/gif", buflen);
    else if (strcasecmp(ext, "svg") == 0) strncpy(out_buf, "image/svg+xml", buflen);
    else if (strcasecmp(ext, "json") == 0) strncpy(out_buf, "application/json", buflen);
    else if (strcasecmp(ext, "txt") == 0) strncpy(out_buf, "text/plain", buflen);
    else if (strcasecmp(ext, "ico") == 0) strncpy(out_buf, "image/x-icon", buflen);
    else if (strcasecmp(ext, "pdf") == 0) strncpy(out_buf, "application/pdf", buflen);
    else if (strcasecmp(ext, "mp4") == 0) strncpy(out_buf, "video/mp4", buflen);
    else if (strcasecmp(ext, "mp3") == 0) strncpy(out_buf, "audio/mpeg", buflen);
    else if (strcasecmp(ext, "xml") == 0) strncpy(out_buf, "text/xml", buflen);
    else strncpy(out_buf, "application/octet-stream", buflen);
    return 0;
}

int serve_static_file(qu_config_t* config, request_ctx_t* ctx) {
    char base_real[PATH_MAX];
    if (realpath((*config).basepath, base_real) == NULL) {
        perror("realpath basepath");
        return -1;
    }

    const char* uri = ctx->uri ? ctx->uri : "/";
    // strip leading '/'
    const char* rel = (uri[0] == '/') ? uri + 1 : uri;

    char candidate_path[PATH_MAX];
    if (rel[0] == '\0') {
        printf("adding index.html\n");
        snprintf(candidate_path, PATH_MAX, "%s/index.html", base_real);
    } else {
        snprintf(candidate_path, PATH_MAX, "%s/%s", base_real, rel);
    }

    struct stat st;
    if (stat(candidate_path, &st) != 0) {
        // Try if it's a directory (when uri ends with /, or server receives path without trailing slash)
        // If candidate is a directory, append index.html
        // Check directory case:
        if (errno == ENOENT) return -1;
        return -1;
    }

    // If path is directory, append index.html
    if (S_ISDIR(st.st_mode)) {
        char idx[PATH_MAX];
        snprintf(idx, sizeof(idx), "%s/index.html", candidate_path);
        if (stat(idx, &st) != 0) return -1;
        strncpy(candidate_path, idx, sizeof(candidate_path));
    }

    char full_real[PATH_MAX];
    if (realpath(candidate_path, full_real) == NULL) return -1;
    size_t base_len = strlen(base_real);
    if (strncmp(full_real, base_real, base_len) != 0) return -1; // directory traversal attempt
    // read file into buffer
    FILE* f = fopen(full_real, "rb");
    if (!f) return -1;
    size_t filesize = (size_t) st.st_size;

    if (filesize > BUFSIZE - 1) {
        fclose(f);
        printf("file too large for response buffer\n");
        return 2; // file too large for response buffer
    }

    // Simple approach: allocate buffer, read entire file.
    // Note: for very large files this will use lots of memory; streaming would be better.
    char* buf = malloc(filesize + 1);
    if (!buf) { fclose(f); return -1; }
    size_t got = fread(buf, 1, filesize, f);
    fclose(f);
    buf[got] = '\0';

    char mime[64];
    get_mime_type(full_real, mime, sizeof(mime));

    ctx->status = 200;
    write_response(ctx, buf, mime);

    free(buf);
    return 0;
}

int serve_large_file(qu_config_t* config, request_ctx_t* ctx) {
    char base_real[PATH_MAX];
    if (realpath((*config).basepath, base_real) == NULL) {
        perror("realpath basepath");
        return -1;
    }

    const char* uri = ctx->uri ? ctx->uri : "/";
    // strip leading '/'
    const char* rel = (uri[0] == '/') ? uri + 1 : uri;

    char candidate_path[PATH_MAX];
    snprintf(candidate_path, PATH_MAX, "%s/%s", base_real, rel);

    struct stat sb;
    int fd = open(candidate_path, O_RDONLY);
    fstat(fd, &sb);
    char header[256];
    char mime[64];
    get_mime_type(candidate_path, mime, sizeof(mime));
    int header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %lld\r\n"
        "\r\n",
        mime,
        (long long)sb.st_size
    );
    write(ctx->sockfd, header, header_len);
    #ifdef __linux__
    // linux sendfile
    off_t offset = 0;
    ssize_t bytes_sent = sendfile(ctx->sockfd, fd, &offset, sb.st_size);
    close(fd);

    if (bytes_sent == -1) {
        perror("sendfile");
        ctx->status = 500;
        return -1;
    }
    #endif

    #if defined(__APPLE__)
    // macOS sendfile
    off_t len = sb.st_size;
    int bytes_sent = sendfile(fd, ctx->sockfd, 0, &len, NULL, 0);
    close(fd);
    if (bytes_sent < 0) {
        perror("sendfile");
        ctx->status = 500;
        return -1;
    }
    #endif
    ctx->status = 200;
    return 0;
}

void write_response(request_ctx_t* ctx, char content[], char content_type[]) {
    char status_str_buf[128];
    get_status_string(status_str_buf, (*ctx).status);
    sprintf((*ctx).response, TEMPLATE_STRING, (*ctx).status, status_str_buf, content_type, content);
}