// references:
// https://beej.us/guide/bgnet/pdf/bgnet_a4_c_1.pdf
// man pages
// TODO: logs

#include "logger.h"

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

constexpr size_t SIZE = 1024;
constexpr const char* PORT_STR = "8080";
constexpr char ipv4[INET_ADDRSTRLEN] = "127.0.0.1";    // 16 bytes
constexpr unsigned int BACKLOG = 1;    // 1 client
constexpr const char *welcome_msg = "Hi, From serv";

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
    short status;
    struct sockaddr_storage incoming;
    socklen_t addr_size;
    struct addrinfo hints, *ai;
    int yes = 1;

    struct sigaction psa;
    psa.sa_handler = &stop_handler;
    psa.sa_flags = 0;
    sigaction(SIGINT, &psa, nullptr);


    // open log file
    if (!Logger::instance().open("test.log")) {
        fprintf(stderr, "Failed to open log file");
        std::exit(EXIT_FAILURE);
    }

    // get info about address
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;   // IPv4
    hints.ai_socktype = SOCK_STREAM;  // TCP
    hints.ai_flags = AI_PASSIVE;   // set up address on its own
    if ((status = getaddrinfo(ipv4, PORT_STR, &hints, &ai)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        std::exit(EXIT_FAILURE);
    }

    // open socket
    if ((sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) == -1) {
        fprintf(stderr, "cannot create socket: %s", std::strerror(errno));
        freeaddrinfo(ai);
        std::exit(EXIT_FAILURE);
    }
    fexcept(LEVEL::INFO, "socket successfully created");

    // associate socket with port from addrinfo
    if ((setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))) < 0) {
        // TODO: under 'fail' label with goto, or smth like this
        fprintf(stderr, "setsockopt error: %s", std::strerror(errno));
        close(sock);
        freeaddrinfo(ai);
        std::exit(EXIT_FAILURE);
    }
    if (bind(sock, ai->ai_addr, ai->ai_addrlen) == -1) {
        fprintf(stderr, "failed to bind: %s", std::strerror(errno));
        close(sock);
        freeaddrinfo(ai);
        std::exit(EXIT_FAILURE);
    }

    freeaddrinfo(ai);

    // listen up to socket to queue incoming connections
    if (listen(sock, BACKLOG) < 0) {
        fprintf(stderr, "listen error: %s", std::strerror(errno));
        close(sock);
        std::exit(EXIT_FAILURE);
    }

    // TODO: can i close listening socket if im gonna take only 1 connection
    // accept incoming calls
    addr_size = sizeof(incoming);
    while (run.load()) {
        if ((newsock = accept(sock, (struct sockaddr*)&incoming, &addr_size)) == -1) {
            if (errno == EINTR) break;
            fprintf(stderr, "failed to accept client: %s", std::strerror(errno));
            continue;
        }

        char buffer[SIZE] = {0};
        int recieved;
        puts("connected");
        // handle connection in another loop
        if (run.load()) {
            if ((recieved = recv(newsock, &buffer, SIZE - 1, 0)) <= 0) {
                fprintf(stderr, "failed to recieve bytes");
                if (recieved == 0) {
                    close(newsock);
                    continue;
                } else {
                    fprintf(stderr, ": %s", std::strerror(errno));
                    close(newsock);
                    continue;
                }
            }
            fprintf(stdout, "Recieved: %s", buffer);
            buffer[recieved-1] = '\0';
            char send_buf[SIZE*2] = {0};
            snprintf(send_buf, sizeof(send_buf), "Accepted: %d bytes (%s)", recieved, buffer);
            int len = strlen(send_buf), sent;
            if ((sent = send(newsock, send_buf, len, 0)) < len) {
                fprintf(stderr, "failed to send all data");
                if (sent == -1) {
                    fprintf(stderr, ": %s", std::strerror(errno));
                } else fprintf(stderr, "\n");
            }
        }
        close(newsock);
        puts("disconnected");
    }

    // unreacheable?
    close(sock);
    return EXIT_SUCCESS;
}
