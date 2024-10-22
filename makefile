CC = gcc
CFLAGS = -g
OBJ_DIR = ./obj

all: debug

debug: arena list
### write the compilation commands here ###
	$(CC) $(CFLAGS) $(OBJ_DIR)/list.o $(OBJ_DIR)/arena.o src/main.c -o target/debug

list:
	$(CC) -c ./lib/list.c -o $(OBJ_DIR)/list.o $(CFLAGS)

arena:
	$(CC) -c ./lib/arena_allocator/arena.c -o $(OBJ_DIR)/arena.o $(CFLAGS)

clean:
	rm $(OBJ_DIR)/* target/debug
