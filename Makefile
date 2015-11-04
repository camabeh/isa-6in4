CFLAGS=-std=c99 -pthread
# CFLAGS=-std=c99 -Wall -Wextra

sixtunnel: sixtunnel.c sixtunnel.h
	gcc sixtunnel.c $(CFLAGS) -o sixtunnel

.PHONY: clean

clean:
	rm -f *.o sixtunnel
