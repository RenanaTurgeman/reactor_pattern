#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/poll.h>
#include <stdbool.h>
#include <signal.h>

#include "hashmap.h"

typedef void (*handler_t)(int fd); 

typedef struct {
    bool isRunning;
    struct pollfd* fds;
    handler_t* handlers;
    int fdCount;
    Hashmap* fdToIndex; 
} Reactor;

void sigintHandler(int sig_num);
void* createReactor();
void stopReactor(void*);
void startReactor(void*);
void addFd(void*, int fd, handler_t handler);
void waitFor(void*);
void freeReactor(void*);
void stdinHandler(int fd);
int main();