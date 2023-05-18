
Reactor *createReactor()
{
    Reactor *reactor = malloc(sizeof(Reactor));
    reactor->fd_count = 0;
    reactor->fd_size = 5;
    reactor->fds = malloc(sizeof(*reactor->fds) * reactor->fd_size);
    reactor->handlers = malloc(sizeof(*reactor->handlers) * reactor->fd_size);
    reactor->running = false;
    reactor->thread = 0;
    return reactor;
}

void stopReactor(Reactor *reactor)
{
    if (reactor->running)
    {
        reactor->running = false;

        pthread_cancel(reactor->thread);
        pthread_join(reactor->thread, NULL);
        pthread_detach(reactor->thread);
    }
}

void *reactor_loop(void *arg)
{
    Reactor *reactor = arg;
    while (reactor->running)
    {
        int poll_count = poll(reactor->fds, reactor->fd_count, -1);
        if (poll_count == -1)
        {
            perror("poll");
            stopReactor(reactor);
        }
        else
        {
            for (int i = 0; i < reactor->fd_count; i++)
            {
                if (reactor->fds[i].revents & POLLIN)
                {
                    reactor->handlers[i](reactor->fds[i].fd, NULL);
                }
            }
        }
    }
    return NULL;
}

void startReactor(Reactor *reactor)
{
    reactor->running = true;
    pthread_create(&(reactor->thread), NULL, reactor_loop, reactor);
}

void addFd(Reactor *reactor, int fd, handler_t handler, void *arg)
{
    if (reactor->fd_count == reactor->fd_size)
    {
        reactor->fd_size *= 2;
        reactor->fds = realloc(reactor->fds, sizeof(*reactor->fds) * reactor->fd_size);
        reactor->handlers = realloc(reactor->handlers, sizeof(*reactor->handlers) * reactor->fd_size);
    }
    reactor->fds[reactor->fd_count].fd = fd;
    reactor->fds[reactor->fd_count].events = POLLIN;
    reactor->handlers[reactor->fd_count] = handler;
    reactor->fd_count++;
}

void waitFor(Reactor *reactor)
{
    if (reactor->running)
    {
        pthread_join(reactor->thread, NULL);
    }
}