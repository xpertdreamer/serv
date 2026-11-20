// NOTE: this version is adapted for client-server music generation

#include "util.h"
#include "shell.h"

#include <cerrno>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdbool.h>

#define SH_CLIENT
#define SH_SERVER

cshell*
csh_create(int fd)
{
    cshell *sh = NULL;

    sh = (cshell*)malloc(sizeof(cshell));
    ALLOC_FAIL(sh);
    sh->prompt = ">";
    sh->prompt_len = (uint32_t)strlen(sh->prompt);
    sh->fd = fd;

    // TODO: log creation
    return sh;
}

sshell*
ssh_create(int fd, cmd* commands, size_t len)
{
    PTR_RECIEVE_FAIL_PTR(commands, ssh_create);
    sshell *sh = NULL;

    sh = (sshell*)malloc(sizeof(sshell));
    ALLOC_FAIL(sh);
    sh->fd = fd;
    sh->num_commands = len;
    sh->commands = (cmd*)malloc(sizeof(cmd) * len);
    if (sh->commands == NULL) {
        free(sh);
        return NULL;
    }
    for (size_t i = 0; i < len; ++i) {
        sh->commands[i] = commands[i];
    }
    // TODO: log creation
    return sh;
}

void
csh_destroy(cshell* instance)
{
    if (instance != NULL) {
        // if (instance->commands != NULL) {
        //     free(instance->commands);
        // }
        free(instance);
    }
    // TODO: log destroying
}

void
ssh_destroy(sshell* instance)
{
    if (instance != NULL) {
        if (instance->commands != NULL) {
            free(instance->commands);
        }
        free(instance);
    }
    // TODO: log destroying
}

void
sh_change_prompt(cshell* instance, const char* newpr)
{
    PTR_RECIEVE_FAIL_VOID(instance, sh_change_prompt);
    instance->prompt = newpr;
    instance->prompt_len = (u32)strlen(newpr);
}

int
sh_recv_ueof(int fd, void* buf, size_t size) {
    if (buf == NULL || size == 0) {
        ERROR("cannot recieve buffer for recv");
        errno = EINVAL;
        return -1;
    }
    if (fd == -1) {
        ERROR("cannot establish connection with the server");
        return -1;
    }

    char* dest = (char*)buf;
    size_t total = 0;
    while (total < size) {
        size_t remainder = size - total;
        ssize_t peekeed = recv(fd, dest+total, remainder, MSG_PEEK);
        if (peekeed < 0) {
            if (errno == EINTR) continue;
            return -1;
        } else if (peekeed == 0) return (int)total;

        char* nullterm = (char*)memchr(dest + total, '\0', (size_t)peekeed);
        size_t to_nt = nullterm ? (size_t)(nullterm - (dest + total) + 1) : (size_t)peekeed;

        int n = recv(fd, dest + total, to_nt, 0);
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        } else if (n == 0) return (int)total;

        total += (size_t)n;

        if (nullterm) return (int)total;
    }

    errno = EMSGSIZE;
    return -1;
}

void
sh_exchange(cshell* instance, const char* name) {
    PTR_RECIEVE_FAIL_VOID(instance, sh_exchange);
    if (instance->fd == -1) {
        ERROR("cannot establish connection with the server");
        return;
    }

    int len = strlen(name), sent, recieved;
    char buf[RESPONSE_SIZE];
    if ((sent = send(instance->fd, name, len, 0)) < len) {
        ERROR("failed to send data");
    }
    // TODO: separate function to recieve until linefeed
    if ((recieved = sh_recv_ueof(instance->fd, buf, RESPONSE_SIZE - 1)) == -1) {
        ERROR("failed to recieve bytes");
    }
    buf[recieved] = '\0';
    fprintf(stdout, "%s", buf);
    fflush(stdout);
}

char*
sh_get_input()
{
    char* buf = NULL;
    size_t s = 0;

    ssize_t read;
    if ((read = getline(&buf, &s, stdin)) == CNNOT) {
        // ignore SIGINT & return NULL
        return NULL;
    }
    if (read < 0) {
        if (ferror(stdin))
            ERROR("cannot receive user input");
        free(buf);
        return NULL;
    }
    if (read == EMPTY) {
        ERROR("input is empty");
        free(buf);
        return NULL;
    }

    buf[read - 1] = '\0';

    return buf;
}

bool
csh_daemon(cshell* instance)
{
    if (instance == NULL) {
        ERROR("instance recieve fail");
        return false;
    }

    putchar('\n');
    write(STDOUT_FILENO, instance->prompt, instance->prompt_len);
    putchar(' ');

    char* input = sh_get_input();
    if (input == NULL) {
        return false;
    }
    sh_exchange(instance, input);

    free(input);
    return true;
}

void
ssh_execute(sshell* instance, const char* name)
{
    PTR_RECIEVE_FAIL_VOID(instance, sh_execute);
    for (size_t i = 0; i < instance->num_commands; ++i) {
        cmd cm = instance->commands[i];
        if (cm.handler == NULL) {
            ERROR("cannot execute this command - handler is null-pointer");
            return;
        }
        if (strcmp(cm.name, name) == 0) {
            DEBUG("called %s", name);
            std::string result = instance->commands[i].handler();
            int len = result.size() + 1, sent;
            if ((sent = send(instance->fd, result.c_str(), len, 0)) < len) {
                ERROR("failed to send all data");
                if (sent == -1) {
                    ERROR(": %s", strerror(errno));
                }
            }
            return;
        }
    }
    std::string result = SH_ERROR("cannot find command [%s] in current instance", name);
    int len = result.size() + 1, sent;
    if ((sent = send(instance->fd, result.c_str(), len, 0)) < len) {
        ERROR("failed to send all data");
        if (sent == -1) {
            ERROR(": %s", strerror(errno));
        }
    }
}
