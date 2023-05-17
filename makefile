CC = gcc
CFLAGS = -g -Wall

all: reactor

reactor: reactor.o hashmap.o
	$(CC) $(CFLAGS) -o reactor reactor.o hashmap.o

reactor.o: reactor.c reactor.h hashmap.h
	$(CC) $(CFLAGS) -c reactor.c

hashmap.o: hashmap.c hashmap.h
	$(CC) $(CFLAGS) -c hashmap.c

clean:
	rm -f reactor reactor.o hashmap.o
