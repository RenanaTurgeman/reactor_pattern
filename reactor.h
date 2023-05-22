#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/poll.h>
#include <stdbool.h>
#include <signal.h>
#include <pthread.h>

#include "hashmap.h"

typedef void (*handler_t)(int fd); 

typedef struct {
    bool isRunning;
    struct pollfd* fds;
    int numPoll;
    handler_t* handlers;
    int fdCount;
    Hashmap* fdToIndex;
    pthread_t thread;
} Reactor;


void* createReactor();
void stopReactor(void*);
void startReactor(void*);
void *reactorRun(void *this);
void addFd(void*, int fd, handler_t handler);
void delFd(void* this, int fd);
void waitFor(void*);
void freeReactor(void*);
int main();