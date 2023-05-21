#include "reactor.h" 

// Reactor* globalReactor;

// void sigintHandler(int sig_num) {
//     printf("\nCaught signal %d, cleaning up and exiting...\n", sig_num);
//     stopReactor(globalReactor);
//     freeReactor(globalReactor);
//     exit(0);
// }

void* reactorThread(void* arg) {
    ThreadData* threadData = (ThreadData*)arg;
    Reactor* reactor = threadData->reactor;

    while (reactor->isRunning) {
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

    return NULL;
}

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

    ThreadData* threadData = malloc(sizeof(ThreadData));
    threadData->reactor = reactor;

    reactor->isRunning = true;

    int result = pthread_create(&(threadData->thread), NULL, reactorThread, (void*)threadData);
    if (result != 0) {
        fprintf(stderr, "Failed to create reactor thread\n");
        return;
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
        // Update the hashmap
        // hashmap_remove(reactor->fdToIndex, fd);
        // hashmap_delete(reactor->fdToIndex, fd);
hashmap_remove(reactor->fdToIndex, fd);

    }
}


void waitFor(void* this) {
    printf("waitFor reactor\n");
    ThreadData* threadData = (ThreadData*)this;
    pthread_join(threadData->thread, NULL);
    free(threadData);
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


// A handler function that will be called when stdin is ready for reading
// void stdinHandler(int fd) {
//     printf("stdinHandler reactor\n");
//     char buffer[1024];
//     ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);
//     if (bytes > 0) {
//         buffer[bytes] = '\0';
//         printf("Received: %s", buffer);
//     } else {
//         printf("EOF or read error\n");
//         exit(EXIT_FAILURE);  // Stop the program if we can't read from stdin anymore
//     }
// }

// int main() {
//     signal(SIGINT, sigintHandler);  // Register the signal handler

//     globalReactor = createReactor();
//     addFd(globalReactor, STDIN_FILENO, stdinHandler);
//     printf("Type something and press Enter. It will be echoed back to you.\n");
//     startReactor(globalReactor);
//     freeReactor(globalReactor);
//     return 0;
// }