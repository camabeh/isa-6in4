CFLAGS=-std=c99 -pthread
# CFLAGS=-std=c99 -Wall -Wextra

all: client server client6 server6

client: client.c client.h
	gcc client.c $(CFLAGS) -o client

server: server.c server.h
	gcc server.c $(CFLAGS) -o server

client6: client6.c client6.h
	gcc client6.c $(CFLAGS) -o client6

server6: server6.c server6.h
	gcc server6.c $(CFLAGS) -o server6

sixtunnel: sixtunnel.c sixtunnel.h
	gcc sixtunnel.c $(CFLAGS) -o sixtunnel

.PHONY: clean

clean:
	rm -f *.o server client
