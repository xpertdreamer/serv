#include "log.h"

#include <cerrno>
#include <cstdio>
#include <sys/socket.h>
#include <cstring>

constexpr const char* ps_help = "help";
constexpr size_t BUF_SIZE = 1024;

void
help_handler(int fd) {
    int len = strlen(ps_help), sent, recieved;
    char buf[BUF_SIZE];
    if ((sent = send(fd, ps_help, len, 0)) < len) {
        log_error("failed to send all data");
        if (sent == -1) {
            log_error(": %s", std::strerror(errno));
        }
    }
    if ((recieved = recv(fd, buf, BUF_SIZE-1, 0)) == -1) {
        log_warn("failed to recieve bytes");
        if (recieved > 0) {
            log_warn(": %s", std::strerror(errno));
        }
    }
    buf[recieved] = '\0';
    fprintf(stdout, "%s", buf);
}
