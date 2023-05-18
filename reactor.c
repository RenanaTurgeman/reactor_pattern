#include "reactor.h" 


void* createReactor() {
    printf("creat reactor\n");
    Reactor* reactor = malloc(sizeof(Reactor));
    reactor->isRunning = false;
    reactor->fds = NULL;
    reactor->handlers = NULL;
    reactor->fdCount = 0;
    reactor->fdToIndex = hashmap_create(); // Initialize the hashmap
    return reactor;
}
void stopReactor(void* this) {
    printf("stop reactor\n");
    ((Reactor*)this)->isRunning = false;
}
void startReactor(void* this) {
    printf("start reactor\n");
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
    printf("addFD reactor\n");
    Reactor* reactor = this;
    reactor->fds = realloc(reactor->fds, (reactor->fdCount + 1) * sizeof(struct pollfd));
    reactor->handlers = realloc(reactor->handlers, (reactor->fdCount + 1) * sizeof(handler_t));
    reactor->fds[reactor->fdCount].fd = fd;
    reactor->fds[reactor->fdCount].events = POLLIN;
    reactor->handlers[reactor->fdCount] = handler;
    hashmap_set(reactor->fdToIndex, fd, reactor->fdCount); // Add the fd and its index to the hashmap
    hashmap_print(reactor->fdToIndex);
    reactor->fdCount++;
}
void delFd(void* this, int fd) {
    Reactor* reactor = (Reactor*)this;
    int index = hashmap_get(reactor->fdToIndex, fd);
    if (index >= 0) {
        // Shift the elements after the index
        for (int i = index; i < reactor->fdCount - 1; i++) {
            reactor->fds[i] = reactor->fds[i + 1];
            reactor->handlers[i] = reactor->handlers[i + 1];
        }

        reactor->fdCount--;

hashmap_remove(reactor->fdToIndex, fd);

    }
}
void waitFor(void* this) {
    printf("waitFor reactor\n");
    // If you were to implement a separate thread for the reactor, you would put pthread_join here.
    // As the current implementation doesn't use threads, we can just wait until the reactor is no longer running.
    while (((Reactor*)this)->isRunning) {
        sleep(1);
    }
}
void freeReactor(void* this) {
    printf("free reactor\n");
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