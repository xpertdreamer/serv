// references:
// https://beej.us/guide/bgnet/pdf/bgnet_a4_c_1.pdf
// man pages

#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <cstdio>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>

constexpr const char* PORT_STR = "8080";
constexpr char ipv4[INET_ADDRSTRLEN] = "127.0.0.1";    // 16 bytes
constexpr unsigned int BACKLOG = 1; // 1 client

int
main(void) {
    short status;
    struct sockaddr_storage incoming;
    socklen_t addr_size;
    struct addrinfo hints, *ai;
    struct sockaddr_in sa;
    int sock = -1, newsock = -1;
    int yes = 1;

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

    // associate socket with port from addrinfo
    if (bind(sock, ai->ai_addr, ai->ai_addrlen) == -1) {
        fprintf(stderr, "failed to bind: %s", std::strerror(errno));
        close(sock);
        freeaddrinfo(ai);
        std::exit(EXIT_FAILURE);
    }
    if ((setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))) < 0) {
        // TODO: under 'fail' label with goto, or smth like this
        fprintf(stderr, "setsockopt error: %s", std::strerror(errno));
        close(sock);
        freeaddrinfo(ai);
        std::exit(EXIT_FAILURE);
    }

    // listen up to socket to queue incoming connections
    if (listen(sock, BACKLOG) < 0) {
        fprintf(stderr, "listen error: %s", std::strerror(errno));
        close(sock);
        freeaddrinfo(ai);
        std::exit(EXIT_FAILURE);
    }

    // TODO: can i close listening socket if im gonna take only 1 connection
    // accept incoming calls
    addr_size = sizeof(incoming);
    while (true) {
        if ((newsock = accept(sock, (struct sockaddr*)&incoming, &addr_size)) == -1) {
            fprintf(stderr, "failed to accept client: %s", std::strerror(errno));
            continue;
        }

        // handle connection in another loop
    }


    // TODO: freeaddrinfo(ai); on exit
    return EXIT_SUCCESS;
}
