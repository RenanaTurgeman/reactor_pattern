#include "reactor.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

#define PORT "9034"

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int get_listener_socket(void) {
    int listener;
    int yes = 1;
    int rv;

    struct addrinfo hints, *ai, *p;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }
    
    for (p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) { 
            continue;
        }
        
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    freeaddrinfo(ai);

    if (p == NULL) {
        return -1;
    }

    if (listen(listener, 10) == -1) {
        return -1;
    }

    return listener;
}

void listener_handler(int fd, void* arg) {
    struct sockaddr_storage remoteaddr;
    socklen_t addrlen;
    char remoteIP[INET6_ADDRSTRLEN];
    int newfd;

    addrlen = sizeof remoteaddr;
    newfd = accept(fd, (struct sockaddr *)&remoteaddr, &addrlen);

    if (newfd == -1) {
        perror("accept");
    } else {
        printf("server: new connection from %s on socket %d\n",
               inet_ntop(remoteaddr.ss_family,
                         get_in_addr((struct sockaddr*)&remoteaddr),
                         remoteIP, INET6_ADDRSTRLEN),
               newfd);
    }
}

void client_handler(int fd, void* arg) {
    char buf[256];
    int nbytes;

    nbytes = recv(fd, buf, sizeof buf, 0);

    if (nbytes <= 0) {
        if (nbytes == 0) {
            printf("server: socket %d hung up\n", fd);
        } else {
            perror("recv");
        }
        close(fd);
    } else {
        printf("server: message from socket %d: %s\n", fd, buf);
    }
}

int main(void) {
    Reactor* reactor = createReactor();
    int listener = get_listener_socket();
    if (listener == -1) {
        fprintf(stderr, "error getting listening socket\n");
        exit(1);
    }

    addFd(reactor, listener, listener_handler, NULL);

    startReactor(reactor);

    waitFor(reactor);

    return 0;
}