#include "reactor.h"

void *createReactor()
{
    printf("create reactor\n");
    Reactor *reactor = malloc(sizeof(Reactor));
    reactor->isRunning = false;
    reactor->fds = NULL;
    reactor->handlers = NULL;
    reactor->fdCount = 1; // Changed from 0 to 1
    reactor->fdToIndex = hashmap_create();
    reactor->thread = 0;
    return reactor;
}

void stopReactor(void *this)
{
    printf("stop reactor\n");
    Reactor *reactor = this;
    if (reactor->isRunning)
    {
        reactor->isRunning = false;

        pthread_cancel(reactor->thread);
        pthread_join(reactor->thread, NULL);
        pthread_detach(reactor->thread);
    }
}

void startReactor(void *this)
{
    printf("start reactor\n");
    Reactor *reactor = this;
    reactor->isRunning = true;
    pthread_create(&reactor->thread, NULL, reactorRun, reactor);
}

void *reactorRun(void *this)
{
    printf("start reactor\n");
    Reactor *reactor = this;
    reactor->isRunning = true;

    while (reactor->isRunning)
    {
        int readyCount = poll(reactor->fds, reactor->fdCount, -1); 
        if (readyCount <= 0)
        {
            continue;
        }
        for (int i = 0; i < reactor->fdCount; i++)
        {
            if (reactor->fds[i].revents & POLLIN)
            {
                reactor->handlers[i](reactor->fds[i].fd);
            }
            else if (reactor->fds[i].revents & (POLLHUP | POLLNVAL | POLLERR))
            {
                delFd(reactor, reactor->fds[i].fd);
            }
        }
    }
    printf("Reactor thread finished.\n");
    return reactor;
}

void addFd(void *this, int fd, handler_t handler)
{
    printf("Adding fd %d with handler %p\n", fd, handler);
    printf("addFD reactor\n");
    Reactor *reactor = this;
    reactor->fds = realloc(reactor->fds, (reactor->fdCount + 1) * sizeof(struct pollfd));
    reactor->handlers = realloc(reactor->handlers, (reactor->fdCount + 1) * sizeof(handler_t));
    reactor->fds[reactor->fdCount].fd = fd;
    reactor->fds[reactor->fdCount].events = POLLIN;
    reactor->handlers[reactor->fdCount] = handler;
    hashmap_set(reactor->fdToIndex, fd, reactor->fdCount);
    hashmap_print(reactor->fdToIndex);
    reactor->fdCount++;
}

void delFd(void *this, int fd)
{
    Reactor *reactor = (Reactor *)this;
    int index = hashmap_get(reactor->fdToIndex, fd);
    if (index >= 0)
    {
        for (int i = index; i < reactor->fdCount - 1; i++)
        {
            reactor->fds[i] = reactor->fds[i + 1];
            reactor->handlers[i] = reactor->handlers[i + 1];
        }
        reactor->fdCount--;
        hashmap_remove(reactor->fdToIndex, fd);
    }
}

void waitFor(void *this)
{
    printf("waitFor reactor\n");
    Reactor *reactor = (Reactor *)this;
    if (!reactor->isRunning)
        return;

    pthread_join(reactor->thread, NULL);
}

void freeReactor(void *this)
{
    printf("free reactor\n");
    Reactor *reactor = this;
    hashmap_free(reactor->fdToIndex);
    if (reactor->fds != NULL)
    {
        free(reactor->fds);
    }
    if (reactor->handlers != NULL)
    {
        free(reactor->handlers);
    }
    free(reactor);
}





// #include "reactor.h"

// void *createReactor()
// {
//     printf("creat reactor\n");
//     Reactor *reactor = malloc(sizeof(Reactor));
//     reactor->isRunning = false;
//     reactor->fds = NULL;
//     reactor->handlers = NULL;
//     reactor->fdCount = 0; 
//     reactor->fdToIndex = hashmap_create();
//     reactor->thread = 0;
//     return reactor;
// }

// void stopReactor(void *this)
// {
//     printf("stop reactor\n");
//     Reactor *reactor = this;
//     if (reactor->isRunning)
//     {
//         reactor->isRunning = false;

//         pthread_cancel(reactor->thread);
//         pthread_join(reactor->thread, NULL);
//         pthread_detach(reactor->thread);
//     }
// }

// void startReactor(void *this)
// {
//     printf("start reactor\n");
//     Reactor *reactor = this;
//     reactor->isRunning = true;
//     pthread_create(&reactor->thread, NULL, reactorRun, reactor);
// }

// void *reactorRun(void *this)
// {
//     printf("start reactor\n");
//     Reactor *reactor = this;
//     reactor->isRunning = true;

//     while (reactor->isRunning)
//     {
//         int readyCount = poll(reactor->fds, reactor->fdCount, -1); ////////// 5000
//         if (readyCount <= 0)
//         {
//             continue;
//         }
//         for (int i = 0; i < reactor->fdCount; i++)//////////////////////
//         {
//             if (reactor->fds[i + 1].revents & POLLIN)
//             {
//                 reactor->handlers[i](reactor->fds[i + 1].fd); // , reactor
//             }
//             else if (reactor->fds[i + 1].revents & (POLLHUP | POLLNVAL | POLLERR))
//             {
//                 delFd(reactor, reactor->fds[i + 1].fd);
//             }
//         }
//     }
//     printf("Reactor thread finished.\n");
//     return reactor;
// }

// void addFd(void *this, int fd, handler_t handler)
// {
//     printf("Adding fd %d with handler %p\n", fd, handler);
//     printf("addFD reactor\n");
//     Reactor *reactor = this;
//     reactor->fds = realloc(reactor->fds, (reactor->fdCount + 1) * sizeof(struct pollfd));
//     reactor->handlers = realloc(reactor->handlers, (reactor->fdCount + 1) * sizeof(handler_t));
//     reactor->fds[reactor->fdCount].fd = fd;
//     reactor->fds[reactor->fdCount].events = POLLIN;
//     reactor->handlers[reactor->fdCount] = handler;
//     hashmap_set(reactor->fdToIndex, fd, reactor->fdCount); // Add the fd and its index to the hashmap
//     hashmap_print(reactor->fdToIndex);
//     reactor->fdCount++;
// }
// void delFd(void *this, int fd)
// {
//     Reactor *reactor = (Reactor *)this;
//     int index = hashmap_get(reactor->fdToIndex, fd);
//     if (index >= 0)
//     {
//         // Shift the elements after the index
//         for (int i = index; i < reactor->fdCount - 1; i++)
//         {
//             reactor->fds[i] = reactor->fds[i + 1];
//             reactor->handlers[i] = reactor->handlers[i + 1];
//         }

//         reactor->fdCount--;

//         hashmap_remove(reactor->fdToIndex, fd);
//     }
// }
// void waitFor(void *this)
// {
//     printf("waitFor reactor\n");
//     Reactor *reactor = (Reactor *)this;
//     if (!reactor->isRunning)
//         return;

//     pthread_join(reactor->thread, NULL);
// }
// void freeReactor(void *this)
// {
//     printf("free reactor\n");
//     Reactor *reactor = this;
//     hashmap_free(reactor->fdToIndex);
//     if (reactor->fds != NULL)
//     {
//         free(reactor->fds);
//     }
//     if (reactor->handlers != NULL)
//     {
//         free(reactor->handlers);
//     }
//     free(reactor);
// }