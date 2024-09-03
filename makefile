CC = gcc
CFLAGS = -Wall
LDFLAGS = -pthread -lrt

.PHONY: all clean

all: broadcaster

broadcaster: broadcaster.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o broadcaster broadcaster.c

clean:
	rm -f broadcaster
