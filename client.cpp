// TODO: reorginize a little bit
// TODO: multithreading or async

#include "log.h"
#include "shell.h"

#include <arpa/inet.h>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string>
#include <chrono>
#include <format>

constexpr std::string logdir = "logs/";

extern void help_handler(int);

enum class ERRORS {
    CON_FAIL = 2,
    SH_ALLOC = 3,
};

int sock = -1;
bool run = true;

extern "C" void
stop_handler(int) {
    run = false;
    std::fflush(stdout);
}

// TODO: into util
const std::string
get_time() {
    auto now = std::chrono::system_clock::now();
    const std::string result = std::format("{:%Y-%m-%d_%H:%M:%S}", now);
    return result;
}

int
main(int argc, char** argv) {
    FILE *flog = NULL;
    struct addrinfo hints, *ai, *p;
    short status;
    char ip4str[INET_ADDRSTRLEN];
    std::string logfile_name = logdir + get_time() + "_client.log";

    cmd cmds[1] = {{"help", help_handler}};

    struct sigaction psa;
    psa.sa_handler = &stop_handler;
    psa.sa_flags = 0;
    sigaction(SIGINT, &psa, nullptr);

    if (argc < 3) {
        puts("Usage: ./client hostname port");
        std::exit(EXIT_FAILURE);
    }

    // open log file
    if ((flog = fopen(logfile_name.c_str(), "w")) == NULL) {
        fprintf(stderr, "Failed to open log file");
        std::exit(EXIT_FAILURE);
    }
    log_add_fp(flog, LOG_INFO);

    // get info about address
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;   // IPv4
    hints.ai_socktype = SOCK_STREAM;   // TCP
    hints.ai_flags = AI_PASSIVE;
    if ((status = getaddrinfo(argv[1], argv[2], &hints, &ai)) != 0) {
        log_error("getaddrinfo error: %s", gai_strerror(status));
        fclose(flog);
        std::exit(EXIT_FAILURE);
    }

    // TODO: check run
    for (p = ai; p != NULL; p = p->ai_next) {
        // open socket
        if ((sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) == -1) {
            log_error( "cannot create socket: %s", std::strerror(errno));
            continue;
        }

        inet_ntop(p->ai_family, &((struct sockaddr_in*)ai->ai_addr)->sin_addr, ip4str, sizeof(ip4str));
        log_info("attempt to connect to %s", ip4str);

        if (connect(sock, p->ai_addr, p->ai_addrlen) == -1) {
            log_error("cannot connect: continue");
            close(sock);
            continue;
        }
        log_info("connected to %s", ip4str);
        break;
    }
    if (p == NULL) {
        log_error("failed to connect");
        fclose(flog);
        freeaddrinfo(ai);
        std::exit((int)ERRORS::CON_FAIL);
    }
    freeaddrinfo(ai);

    shell *sh = sh_create(cmds, 1, sock);
    if (sh == NULL) {
        log_error("cannot initialize shell");
        fclose(flog);
        std::exit((int)ERRORS::SH_ALLOC);
    }

    while(sh_daemon(sh) && run);

    sh_destroy(sh);
    close(sock);
    fclose(flog);
    return EXIT_SUCCESS;
}
