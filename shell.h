#ifndef SHELL_H
#define SHELL_H

// Usage:
// cmd cmds[1] = {{"help", "description", help_handler}}; // initialize vector of commands
// shell *sh = sh_create(cmds, 1);                        // create new shell
// sh_change_prompt(sh, '>');                             // change standard propmt if neededx
// while(sh_daemon(sh));                                  // run daemon to listen user input and execute
// sh_destroy(sh);                                        // free memory on exit

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RESPONSE_SIZE 1024

typedef enum {
    EXCHANGE = 0,
    EXECUTE,
} sh_type;

typedef enum sh_input_error {
    CNNOT = -1,
    EMPTY = 1,
} sh_ierr;

typedef struct Shell shell;

typedef uint16_t u32;

typedef struct {
    const char* name;
    void (*handler)();
} cmd;


// TODO: struct for num commanfs and commands -> union with fd
struct Shell {
    const char* prompt;
    size_t num_commands;
    cmd* commands;
    u32 prompt_len;
    int fd;
    sh_type type;
};

shell*
sh_create(cmd* commands, size_t len, int fd);

void
sh_destroy(shell* instance);

void
sh_change_prompt(shell* instance, const char* newpr);

void
sh_execute(shell* instance, const char* name);

void
sh_exhange(shell* instance, const char* name);

bool
sh_daemon(shell* instance);

char*
sh_get_input();

#ifdef __cplusplus
}
#endif
#endif
