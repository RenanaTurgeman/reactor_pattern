#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>

#define PORT "9034"

typedef void (*handler_t)(int fd);

void addFd(struct reactor_t *reactor, int fd, handler_t handler);

// Return a listening socket
int get_listener_socket(void)
{
    int listener;     // Listening socket descriptor
    int yes=1;        // For setsockopt() SO_REUSEADDR, below
    int rv;

    struct addrinfo hints, *ai, *p;

    // Get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }
    
    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) { 
            continue;
        }
        
        // Lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    freeaddrinfo(ai); // All done with this

    // If we got here, it means we didn't get bound
    if (p == NULL) {
        return -1;
    }

    // Listen
    if (listen(listener, 10) == -1) {
        return -1;
    }

    return listener;
}

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

struct reactor_t {
    struct pollfd *fds;
    handler_t *handlers;
    int fd_count;
    int fd_capacity;
};

struct reactor_t* createReactor(int listener) {
    struct reactor_t *reactor = malloc(sizeof(struct reactor_t));
    reactor->fds = malloc(sizeof(struct pollfd) * 10);
    reactor->handlers = malloc(sizeof(handler_t) * 10);
    reactor->fd_count = 0;
    reactor->fd_capacity = 10;

    // Adding the listener fd to the reactor
    addFd(reactor, listener, handle_new_connection);

    return reactor;
}

void addFd(struct reactor_t *reactor, int fd, handler_t handler) {
    if (reactor->fd_count == reactor->fd_capacity) {
        reactor->fd_capacity *= 2;
        reactor->fds = realloc(reactor->fds, sizeof(struct pollfd) * reactor->fd_capacity);
        reactor->handlers = realloc(reactor->handlers, sizeof(handler_t) * reactor->fd_capacity);
    }
    reactor->fds[reactor->fd_count].fd = fd;
    reactor->fds[reactor->fd_count].events = POLLIN;
    reactor->handlers[reactor->fd_count] = handler;
    reactor->fd_count++;
}

void startReactor(struct reactor_t *reactor) {
    while (1) {
        int ready = poll(reactor->fds, reactor->fd_count, -1);
        if (ready == -1) {
            perror("poll");
            exit(1);
        }
        for (int i = 0; i < reactor->fd_count; i++) {
            if (reactor->fds[i].revents & POLLIN) {
                reactor->handlers[i](reactor->fds[i].fd);
            }
        }
    }
}

void stopReactor(struct reactor_t *reactor) {
    free(reactor->fds);
    free(reactor->handlers);
    free(reactor);
}

void handle_new_connection(int fd) {
    // Implement your logic for handling a new connection
    printf("Handling new connection on fd: %d\n", fd);
}

void handle_client_message(int fd) {
    // Implement your logic for handling a client message
    printf("Handling client message on fd: %d\n", fd);
}

int main() {
    int listener = get_listener_socket();  // You'll need to implement this

    if (listener == -1) {
        fprintf(stderr, "error getting listening socket\n");
        exit(1);
    }

    struct reactor_t* reactor = createReactor(listener);
    startReactor(reactor);
    stopReactor(reactor);
    return 0;
}
