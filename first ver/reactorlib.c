#include <poll.h>
#include <stdlib.h>
#include <pthread.h>

typedef void (*handler_t)(int);

typedef struct {
    int fd;
    handler_t handler;
} FdHandler;

typedef struct {
    struct pollfd *pfds;
    FdHandler *fdHandlers;
    int fd_count;
    int fd_size;
    int running;
    pthread_t thread;
} Reactor;

void* reactor_loop(void* arg) {
    Reactor reactor = (Reactor)arg;
    while (reactor->running) {
        int poll_count = poll(reactor->pfds, reactor->fd_count, -1);
        if (poll_count == -1) {
            perror("poll");
            exit(1);
        }
        for (int i = 0; i < reactor->fd_count; i++) {
            if (reactor->pfds[i].revents & POLLIN) {
                reactor->fdHandlers[i].handler(reactor->pfds[i].fd);
            }
        }
    }
    return NULL;
}

void* createReactor() {
    Reactor *reactor = malloc(sizeof(Reactor));
    reactor->fd_count = 0;
    reactor->fd_size = 5;
    reactor->pfds = malloc(sizeof(struct pollfd) * reactor->fd_size);
    reactor->fdHandlers = malloc(sizeof(FdHandler) * reactor->fd_size);
    reactor->running = 0;
    return reactor;
}

void stopReactor(void *this) {
    Reactor reactor = (Reactor)this;
    reactor->running = 0;
}

void startReactor(void *this) {
    Reactor reactor = (Reactor)this;
    reactor->running = 1;
    if (pthread_create(&reactor->thread, NULL, reactor_loop, reactor) != 0) {
        perror("pthread_create");
        exit(1);
    }
}

void addFd(void *this, int fd, handler_t handler) {
    Reactor reactor = (Reactor)this;
    if (reactor->fd_count == reactor->fd_size) {
        reactor->fd_size *= 2;
        reactor->pfds = realloc(reactor->pfds, sizeof(struct pollfd) * reactor->fd_size);
        reactor->fdHandlers = realloc(reactor->fdHandlers, sizeof(FdHandler) * reactor->fd_size);
    }
    reactor->pfds[reactor->fd_count].fd = fd;
    reactor->pfds[reactor->fd_count].events = POLLIN;
    reactor->fdHandlers[reactor->fd_count].fd = fd;
    reactor->fdHandlers[reactor->fd_count].handler = handler;
    reactor->fd_count++;
}

void waitFor(void *this) {
    Reactor reactor = (Reactor)this;
    pthread_join(reactor->thread, NULL);
}