CC = gcc
CFLAGS = -g
OBJ_DIR = ./obj

all: debug

debug: arena
### write the compilation commands here ###
	$(CC) $(CFLAGS) $(OBJ_DIR)/arena.o src/main.c -o target/debug

arena:
	$(CC) -c ./lib/arena_allocator/arena.c -o $(OBJ_DIR)/arena.o $(CFLAGS)

clean:
	rm $(OBJ_DIR)/* debug/target
