CC = gcc
CFLAGS = -g -Wall
LIBS = -lpthread
SHAREDLIB = -shared
RM = rm -f

all: react_server

react_server: react_server.o libreactor.so hashmap.o
	$(CC) $(CFLAGS) -o react_server react_server.o hashmap.o -L. -lreactor

react_server.o: server.c reactor.h
	$(CC) $(CFLAGS) -c server.c -o react_server.o

libreactor.so: reactor.o
	$(CC) $(CFLAGS) $(SHAREDLIB) -o libreactor.so reactor.o

reactor.o: reactor.c reactor.h hashmap.h
	$(CC) $(CFLAGS) -fPIC -c reactor.c

hashmap.o: hashmap.c hashmap.h
	$(CC) $(CFLAGS) -c hashmap.c

clean:
	$(RM) react_server react_server.o reactor.o hashmap.o libreactor.so






# CC = gcc
# CFLAGS = -g -Wall
# LIBS = -lpthread
# SHAREDLIB = -shared
# RM = rm -f

# all: server

# server: server.o libreactor.so hashmap.o
# 	$(CC) $(CFLAGS) -o server server.o hashmap.o -L. -lreactor

# server.o: server.c reactor.h
# 	$(CC) $(CFLAGS) -c server.c

# libreactor.so: reactor.o
# 	$(CC) $(CFLAGS) $(SHAREDLIB) -o libreactor.so reactor.o

# reactor.o: reactor.c reactor.h hashmap.h
# 	$(CC) $(CFLAGS) -fPIC -c reactor.c

# hashmap.o: hashmap.c hashmap.h
# 	$(CC) $(CFLAGS) -c hashmap.c

# clean:
# 	$(RM) server server.o reactor.o hashmap.o libreactor.so


