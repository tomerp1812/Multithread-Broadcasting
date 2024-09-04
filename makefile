CC = gcc
CFLAGS = -Wall
LDFLAGS = -pthread -lrt
SRC = main.c threads_manager.c bounded.c unbounded.c
OBJ = $(SRC:.c=.o)
TARGET = broadcaster

.PHONY: all clean

all: $(TARGET)

# Link object files to create the final executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJ)

# Compile each source file into object files
%.o: %.c bounded.h unbounded.h threads_manager.h broadcaster.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
