#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/poll.h>
#include <stdbool.h>

#include "hashmap.h" // An external hashmap library

typedef void (*handler_t)(int fd); 

typedef struct {
    bool isRunning;
    struct pollfd* fds;
    handler_t* handlers;
    int fdCount;
    Hashmap* fdToIndex; // Maps a file descriptor to its index in fds and handlers arrays
} Reactor;

void* createReactor() {
    Reactor* reactor = malloc(sizeof(Reactor));
    reactor->isRunning = false;
    reactor->fds = NULL;
    reactor->handlers = NULL;
    reactor->fdCount = 0;
    reactor->fdToIndex = hashmap_create(); // Initialize the hashmap
    return reactor;
}

void stopReactor(void* this) {
    ((Reactor*)this)->isRunning = false;
}

void startReactor(void* this) {
    Reactor* reactor = this;
    reactor->isRunning = true;
    while(reactor->isRunning) {
        int readyCount = poll(reactor->fds, reactor->fdCount, -1);
        if (readyCount <= 0) {
            continue;
        }

        for (int i = 0; i < reactor->fdCount; i++) {
            if (reactor->fds[i].revents & POLLIN) {
                reactor->handlers[i](reactor->fds[i].fd);
            }
        }
    }
}

void addFd(void* this, int fd, handler_t handler) {
    Reactor* reactor = this;
    reactor->fds = realloc(reactor->fds, (reactor->fdCount + 1) * sizeof(struct pollfd));
    reactor->handlers = realloc(reactor->handlers, (reactor->fdCount + 1) * sizeof(handler_t));

    reactor->fds[reactor->fdCount].fd = fd;
    reactor->fds[reactor->fdCount].events = POLLIN;
    reactor->handlers[reactor->fdCount] = handler;

    hashmap_set(reactor->fdToIndex, fd, reactor->fdCount); // Add the fd and its index to the hashmap

    reactor->fdCount++;
}

void waitFor(void* this) {
    // If you were to implement a separate thread for the reactor, you would put pthread_join here.
    // As the current implementation doesn't use threads, we can just wait until the reactor is no longer running.
    while (((Reactor*)this)->isRunning) {
        sleep(1);
    }
}
void freeReactor(void* this) {
    Reactor* reactor = this;
    hashmap_free(reactor->fdToIndex);

    if (reactor->fds != NULL) {
        free(reactor->fds);
    }
    if (reactor->handlers != NULL) {
        free(reactor->handlers);
    }

    free(reactor);
}

// A handler function that will be called when stdin is ready for reading
void stdinHandler(int fd) {
    char buffer[1024];
    ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes > 0) {
        buffer[bytes] = '\0';
        printf("Received: %s", buffer);
    } else {
        printf("EOF or read error\n");
        exit(EXIT_FAILURE);  // Stop the program if we can't read from stdin anymore
    }
}

int main() {
    Reactor* reactor = createReactor();
    addFd(reactor, STDIN_FILENO, stdinHandler);
    printf("Type something and press Enter. It will be echoed back to you.\n");
    startReactor(reactor);
    freeReactor(reactor);
    return 0;
}


