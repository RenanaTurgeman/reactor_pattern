# Reactor Chat Server

The Reactor Chat Server is a chat server implemented using the reactor pattern and the poll mechanism. It supports an unlimited number of customers and runs in a single thread.

## Getting Started

### Prerequisites
To compile and run the server, you need to have the following dependencies installed:

- C compiler (e.g., GCC)

### Building
Clone the repository:
```bash
git clone https://github.com/RenanaTurgeman/reactor_pattern.git

# Building all the necessary files & the main programs.
make all
```

## Running the Server

To run the server, follow these steps:

1. Set the library path:

```bash
export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH
```
Execute the compiled binary:
```bash
./react_server
```
The server will start listening for incoming connections on port 9034. Clients can connect to the server using Telnet or any other TCP client.


## Usage

Once the server is running, clients can connect to it using a TCP client. For example, you can use Telnet:

```bash
telnet 127.0.0.1 9034
```
Clients can send messages to the server, which will be echoed back to them.

## Design
The server follows the reactor design pattern, which allows it to handle multiple file descriptors efficiently using a single thread. The server uses the poll/select mechanism to listen for incoming data on multiple file descriptors simultaneously.

The main components of the server are:

react_server.c: The main entry point of the server. It sets up the necessary components, such as signal handlers, and starts the reactor.
reactor.h and reactor.c: Implements the reactor structure and its related functions. The reactor manages the file descriptors and their associated event handlers. It uses the poll/select mechanism to wait for incoming data and dispatches the appropriate event handlers.
hashmap.h and hashmap.c: A hashmap implementation used by the reactor to map file descriptors to their corresponding event handlers.

