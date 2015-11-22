sixtunnel: main.c main.h send4.c send4.h listen6.c listen6.h listen4.c listen4.h logger.c logger.h
	gcc -std=c99 *.c -pthread -o sixtunnel

.PHONY: clean

clean:
	rm *.o sixtunnel -f