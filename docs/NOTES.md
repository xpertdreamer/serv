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
