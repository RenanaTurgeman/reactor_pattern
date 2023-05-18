
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
#include "reactor.h"

int handler_client(int fd);

#define PORT "9034" // Port we're listening on

Reactor *globalReactor;

void sigintHandler(int sig_num)
{
    printf("\nCaught signal %d, cleaning up and exiting...\n", sig_num);
    stopReactor(globalReactor);
    freeReactor(globalReactor);
    exit(0);
}

// Get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

// Return a listening socket
int get_listener_socket(void)
{
    int listener; // Listening socket descriptor
    int yes = 1;  // For setsockopt() SO_REUSEADDR, below
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

        // Lose the pesky "address already in use" error message
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

// Add a new file descriptor to the set
void add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size)
{
    // If we don't have room, add more space in the pfds array
    if (*fd_count == *fd_size)
    {
        *fd_size *= 2; // Double it

        *pfds = realloc(*pfds, sizeof(**pfds) * (*fd_size));
    }

    (*pfds)[*fd_count].fd = newfd;
    (*pfds)[*fd_count].events = POLLIN; // Check ready-to-read

    (*fd_count)++;
}

// Remove an index from the set
void del_from_pfds(struct pollfd pfds[], int i, int *fd_count)
{
    // Copy the one from the end over this one
    pfds[i] = pfds[*fd_count - 1];

    (*fd_count)--;
}

int handler_server(int listener) // handler_server
{
    int newfd;                          // Newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // Client address
    socklen_t addrlen;

    char buf[256]; // Buffer for client data

    char remoteIP[INET6_ADDRSTRLEN];

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
        addFd(globalReactor, newfd, &handler_client);
    }
}

int handler_client(int fd)
{
    char buf[256];
    int nbytes = recv(fd, buf, sizeof(buf) - 1, 0);

    if (nbytes <= 0)
    {
        // Got error or connection closed by client
        if (nbytes == 0)
        {
            // Connection closed
            printf("pollserver: socket %d hung up\n", fd);
        }
        else
        {
            perror("recv");
        }

        close(fd); // Bye!
        delFd(globalReactor, fd);
    }
    else
    {
        // We got some good data from the client
        buf[nbytes] = '\0'; // Null-terminate the data
        printf("Received data: %s\n", buf);
    }
    return 0;
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

// void test2(int fd)
// {
//     char tv = getchar();
//     while (tv != '\n')
//     {
//         printf("%c", tv);
//         tv = getchar();
//     }
//     printf("\n");
// }

// int main()
// {
//     Reactor *r = createReactor();
//     addFd(r, 0, &test2);
//     startReactor(r);
//     while (1)
//     {
//         /* code */
//     }

//     return 0;
// }