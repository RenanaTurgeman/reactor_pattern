



# CC := gcc
# CFLAGS := -Wall -Werror -fpic

# LIBRARY := libreactor.so
# LIB_SOURCES := reactor.c hashmap.c
# LIB_OBJECTS := $(LIB_SOURCES:.c=.o)

# all: $(LIBRARY)

# $(LIBRARY): $(LIB_OBJECTS)
# 	$(CC) -shared -o $@ $^

# %.o: %.c
# 	$(CC) $(CFLAGS) -c $< -o $@

# clean:
# 	rm -f $(LIBRARY) $(LIB_OBJECTS)



CC = gcc
CFLAGS = -g -Wall

all: server

server: server.o reactor.o hashmap.o
	$(CC) $(CFLAGS) -o server server.o reactor.o hashmap.o

server.o: server.c reactor.h
	$(CC) $(CFLAGS) -c server.c

reactor.o: reactor.c reactor.h hashmap.h
	$(CC) $(CFLAGS) -c reactor.c

hashmap.o: hashmap.c hashmap.h
	$(CC) $(CFLAGS) -c hashmap.c

clean:
	rm -f server server.o reactor.o hashmap.o
