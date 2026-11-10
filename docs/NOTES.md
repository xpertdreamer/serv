Need to be called for the client:

```c++
    #include <sys/socket.h>

    int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

Basically like this:

```c++
    if (connect(sock, ai->ai_addr, ai->ai_addrlen) < 0) {
        fprintf(stderr, "connection error: %s", std::strerror(errno));
        close(sock);
        freeaddrinfo(ai);
        std::exit(EXIT_FAILURE);
    }
```

To listen incoming connections the sequence of syscalls will be:

```c++
getaddrinfo();
socket();
bind();
listen();
/* accept() goes here */
```

According to [Beej’s Guide to Network Programming](https://beej.us/guide/bgnet/pdf/bgnet_a4_c_1.pdf)


