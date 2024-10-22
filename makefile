CC = gcc
CFLAGS = -g
OBJ_DIR = ./obj

all: debug

debug: arena linked_list
### write the compilation commands here ###
	$(CC) $(CFLAGS) $(OBJ_DIR)/linked_list.o $(OBJ_DIR)/arena.o src/main.c -o target/debug

linked_list:
	$(CC) -c ./lib/linked_list.c -o $(OBJ_DIR)/linked_list.o $(CFLAGS)

arena:
	$(CC) -c ./lib/arena_allocator/arena.c -o $(OBJ_DIR)/arena.o $(CFLAGS)

clean:
	rm $(OBJ_DIR)/* debug/target
