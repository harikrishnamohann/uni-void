CC = gcc
CFLAGS = -lncurses
DEBUG = debug1

all: arena
	@ $(CC) src/main.c obj/arena.o -o target/$(DEBUG) $(CFLAGS)
run: arena
	@ $(CC) src/main.c obj/arena.o -o target/$(DEBUG) $(CFLAGS)
	@ ./target/$(DEBUG)

arena: check
	@ $(CC) -c lib/arena_allocator/arena.c  -o obj/arena.o
	
check: ./obj ./target
	
./obj:
	@echo "Creating directory ./obj"
	mkdir -p ./obj

./target:
	@echo "Creating directroy ./target"
	mkdir -p ./target

clean:
	@ rm obj/* target/$(DEBUG)

