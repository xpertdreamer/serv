// NOTE: this version is adapted for client-server music generation
#ifndef SHELL_H
#define SHELL_H

#include "util.h"

#include <atomic>
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline const char*
format_error_impl(const char* fmt, ...) {
    static char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    return buffer;
}

#define SH_ERROR(msg, ...) format_error_impl(ERROR_COLOR msg RESET_COLOR, ##__VA_ARGS__)

typedef enum sh_input_error {
    CNNOT = -1,
    EMPTY = 1,
} sh_ierr;

typedef struct ServerShell sshell;
typedef struct ClientShell cshell;

typedef uint16_t u32;

int
sh_recv_ueof(int fd, void* buf, size_t size);

char*
sh_get_input();

#define RESPONSE_SIZE 256000

// Client shell section
struct ClientShell {
    const char* prompt;
    u32 prompt_len;
    int fd;
};

cshell*
csh_create(int fd);

void
csh_destroy(cshell* instance);

void
csh_change_prompt(cshell* instance, const char* newpr);

void
csh_exhange(cshell* instance, const char* name);

bool
csh_daemon(cshell* instance);

// Server shell section

typedef struct {
    const char* name;
    const char* (*handler)();
} cmd;

// TODO: struct for num commanfs and commands -> union with fd
struct ServerShell {
    cmd* commands;
    size_t num_commands;
    int fd;
};

sshell*
ssh_create(int fd, cmd* commands, size_t len);

void
ssh_destroy(sshell* instance);

void
ssh_execute(sshell* instance, const char* name);

#ifdef __cplusplus
}
#endif
#endif
