CC = gcc
# CFLAGS =
OBJ_DIR = ./obj

all: debug

debug:
### write the compilation commands here ###
	$(CC) $(CFLAGS) src/main.c -o target/debug

arena:
	$(CC) -c ./lib/arena_allocator/arena.c -o $(OBJ_DIR)/arena.o

clean:
	rm $(OBJ_DIR)/*
