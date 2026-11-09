// references:
// https://beej.us/guide/bgnet/pdf/bgnet_a4_c_1.pdf
// man pages

#include <arpa/inet.h>
#include <cstdio>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>

constexpr unsigned long PORT = 8080;

int
main(void) {
    // TODO: rewrite with addrinfo
    char ipv4[INET_ADDRSTRLEN] = "127.0.0.1"; // 16 bytes
    struct addrinfo hints, *ai;
    struct sockaddr_in sa;
    int sock = -1;

    // ...

    // open socket
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_IP)) == -1) {
        fprintf(stderr, "cannot create socket: %s", std::strerror(errno));
        exit(EXIT_FAILURE);
    }

    // setting up IPv4 socket address
    sa.sin_family = AF_INET;
    sa.sin_port = htons(PORT);
    // convert string address to network address representation
    if (inet_pton(AF_INET, ipv4, &(sa.sin_addr)) != 1) {
        fprintf(stderr, "Could to turn ipv4 to network representation");
        close(sock);
        std::exit(EXIT_FAILURE);
    }

    // associate socket with port 'PORT' in sockaddr_in struct
    if (bind(sock, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
        fprintf(stderr, "failed to bind: %s", std::strerror(errno));
        close(sock);
        std::exit(EXIT_FAILURE);
    }


}
