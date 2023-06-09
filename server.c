#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <errno.h>

#include "reactor.h"

#define PORT "9034"

void handler_client(int fd);
int setSocketNonBlocking(int fd); 

Reactor *globalReactor;

void sigintHandler(int sig_num)
{
    printf("\nCaught signal %d, cleaning up and exiting...\n", sig_num);
    stopReactor(globalReactor);
    freeReactor(globalReactor);
    exit(0);
}

// Return a listening socket
int get_listener_socket(void)
{
    int listener;
    int yes = 1;
    int rv;

    struct addrinfo hints, *ai, *p;

    // Get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0)
    {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for (p = ai; p != NULL; p = p->ai_next)
    {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0)
        {
            continue;
        }

        // // Lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0)
        {
            close(listener);
            continue;
        }

        break;
    }

    freeaddrinfo(ai); // All done with this

    // If we got here, it means we didn't get bound
    if (p == NULL)
    {
        return -1;
    }

    // Listen
    if (listen(listener, 10) == -1)
    {
        return -1;
    }

    return listener;
}

void handler_server(int listener)
{
    int newfd;
    struct sockaddr_storage remoteaddr;
    socklen_t addrlen;

    addrlen = sizeof remoteaddr;
    newfd = accept(listener,
                   (struct sockaddr *)&remoteaddr,
                   &addrlen);

    if (newfd == -1)
    {
        perror("accept");
    }
    else
    {
        if (setSocketNonBlocking(newfd) < 0) 
        {
            close(newfd);
            return;
        }
        addFd(globalReactor, newfd, &handler_client);
    }
}


int setSocketNonBlocking(int fd) 
{
    int flags;

    // Get the current flags for the socket
    if ((flags = fcntl(fd, F_GETFL, NULL)) < 0) 
    {
        perror("fcntl");
        return -1;
    }

    // Set the socket to non-blocking
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) 
    {
        perror("fcntl");
        return -1;
    }

    return 0;
}

void handler_client(int fd)
{
    char buf[1024];  // Increase the buffer size
    int nbytes = recv(fd, buf, sizeof(buf) - 1, 0);

    if (nbytes <= 0)
    {
        if (nbytes == 0 || (nbytes == -1 && errno != EAGAIN && errno != EWOULDBLOCK))
        {
            if (nbytes == 0)
            {
                printf("pollserver: socket %d hung up\n", fd);
            }
            else
            {
                perror("recv");
            }

            close(fd);
            delFd(globalReactor, fd);
        }
    }
    else
    {
        buf[nbytes] = '\0';
        printf("\x1b[36m Received data from client %d:\x1b[0m\n \x1b[35m%s\x1b[0m\n", fd-3 , buf);
        
        // Send an acknowledgement message to the client
        const char* msg = "Message received\n";
        send(fd, msg, strlen(msg), 0);  // Don't forget to check for errors in a real scenario!
    }
}



int main()
{

    signal(SIGINT, sigintHandler); // Register the signal handler

    globalReactor = createReactor();
    int listener = get_listener_socket();

    if (listener == -1)
    {
        fprintf(stderr, "error getting listening socket\n"); ////////////////
        exit(1);
    }

    addFd(globalReactor, listener, handler_server);
    startReactor(globalReactor);
    waitFor(globalReactor);
    printf("Type something and press Enter. It will be echoed back to you.\n");
    freeReactor(globalReactor);
    return 0;
}