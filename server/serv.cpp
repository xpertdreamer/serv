// references:
// https://beej.us/guide/bgnet/pdf/bgnet_a4_c_1.pdf
// man pages

// TODO: reorginize a little bit
// TODO: own back shell (executor)
// TODO: multithreading

#include "log.h"
#include "util.h"

#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <atomic>
#include <csignal>
#include <cstdio>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>

#include <string>

constexpr size_t SIZE = 1024;
constexpr const char* PORT_STR = "8080";
constexpr char ipv4[INET_ADDRSTRLEN] = "127.0.0.1";    // 16 bytes localhost
constexpr unsigned int BACKLOG = 1;   // 1 client

std::atomic<int> run = 1;
int sock = -1, newsock = -1;

extern "C" void
stop_handler(int sig) {
    (void)sig;
    run.store(0);
    std::fflush(stdout);
}

int
main(void) {
    FILE* flog = NULL;
    short status;
    struct sockaddr_storage incoming;
    socklen_t addr_size;
    struct addrinfo hints, *ai;
    int yes = 1;
    std::string logfile_name = logdir + get_time() + ".log";

    char buffer[SIZE] = {0};
    int recieved;

    struct sigaction psa;
    psa.sa_handler = &stop_handler;
    psa.sa_flags = 0;
    sigaction(SIGINT, &psa, nullptr);

    // open log file
    if ((flog = fopen(logfile_name.c_str(), "w")) == NULL) {
        fprintf(stderr, "Failed to open log file");
        std::exit(EXIT_FAILURE);
    }
    log_add_fp(flog, LOG_INFO);

    // get info about address
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;   // IPv4
    hints.ai_socktype = SOCK_STREAM;  // TCP
    hints.ai_flags = AI_PASSIVE;   // set up address on its own
    if ((status = getaddrinfo(ipv4, PORT_STR, &hints, &ai)) != 0) {
        log_error("getaddrinfo error: %s", gai_strerror(status));
        fclose(flog);
        std::exit(EXIT_FAILURE);
    }

    // open socket
    if ((sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) == -1) {
        log_error( "cannot create socket: %s", std::strerror(errno));
        freeaddrinfo(ai);
        fclose(flog);
        std::exit(EXIT_FAILURE);
    }
    char ip4[INET_ADDRSTRLEN];
    inet_ntop(ai->ai_family, &((struct sockaddr_in*)ai->ai_addr)->sin_addr, ip4, ai->ai_addrlen);
    log_info("socket successfuly opened: %s", ip4);

    // associate socket with port from addrinfo
    if ((setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))) < 0) {
        // TODO: under 'fail' label with goto, or smth like this
        log_error("setsockopt error: %s", std::strerror(errno));
        close(sock);
        freeaddrinfo(ai);
        fclose(flog);
        std::exit(EXIT_FAILURE);
    }
    if (bind(sock, ai->ai_addr, ai->ai_addrlen) == -1) {
        log_error("failed to bind: %s", std::strerror(errno));
        close(sock);
        freeaddrinfo(ai);
        fclose(flog);
        std::exit(EXIT_FAILURE);
    }
    log_info("socket binded to port %s", PORT_STR);
    freeaddrinfo(ai);

    // listen up to socket to queue incoming connections
    if (listen(sock, BACKLOG) < 0) {
        log_error("listen error: %s", std::strerror(errno));
        close(sock);
        fclose(flog);
        std::exit(EXIT_FAILURE);
    }

    // accept incoming calls
    addr_size = sizeof(incoming);
    // NOTE: To accept clients forever, wrap everything from here to just before newsock close() in a loop
    if ((newsock = accept(sock, (struct sockaddr*)&incoming, &addr_size)) == -1) {
        log_error("failed to accept client: %s", std::strerror(errno));
        if (errno == EINTR) {
            goto end;
        }
    }

    log_info("connected");
    // handle connection in another loop
    while (run.load()) {
        if ((recieved = recv(newsock, &buffer, SIZE - 1, 0)) <= 0) {
            if (recieved < 0 && errno == EINTR) {
                if (!run.load()) break;
                continue;
            }
            if (recieved == 0) log_info("client closed the connection");
            else log_warn("recv failed: %s", std::strerror(errno));
            break;
        }
        buffer[recieved] = '\0';
        log_info("Recieved: %s", buffer);
        char send_buf[SIZE*2] = {0};
        snprintf(send_buf, sizeof(send_buf), "Accepted: %d bytes (%s)", recieved, buffer);
        int len = strlen(send_buf), sent;
        if ((sent = send(newsock, send_buf, len, 0)) < len) {
            log_error("failed to send all data");
            if (sent == -1) {
                log_error(": %s", std::strerror(errno));
            }
        }
    }
    close(newsock);
    newsock = -1;

end:
    log_info("disconnected");
    log_info("shutting down (%s)", get_time().c_str());
    fclose(flog);
    close(sock);
    return EXIT_SUCCESS;
}
