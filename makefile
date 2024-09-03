CC = gcc
CFLAGS = -Wall
LDFLAGS = -pthread -lrt

.PHONY: all clean

all: ex3.out

ex3.out: ex3.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o ex3.out ex3.c

clean:
	rm -f ex3.out
