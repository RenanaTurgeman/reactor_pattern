#ifndef REACTOR_H
#define REACTOR_H

#include <poll.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>



struct Reactor {
    struct pollfd* fds;
    handler_t* handlers;
    int fd_count;
    int fd_size;
    bool running;
    pthread_t thread;
};

typedef struct Reactor Reactor;

typedef void (handler_t)(int fd, void arg);

Reactor* createReactor();
void stopReactor(Reactor* reactor);
void startReactor(Reactor* reactor);
void addFd(Reactor* reactor, int fd, handler_t handler, void* arg);
void waitFor(Reactor* reactor);

#endif