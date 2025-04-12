CC = gcc
CFLAGS = -lncurses -std=c23 -Wall -Werror
DEBUG = debug
RELEASE = uni-void

all: obj target game_files target/$(DEBUG)

install: release
	sudo cp target/$(RELEASE) /usr/bin/$(RELEASE)

release: obj target game_files target/$(RELEASE)

# Run target
run: obj target game_files target/$(DEBUG)
	./target/$(DEBUG)

target/$(RELEASE): obj/main.o obj/lb.o obj/arena.o obj/keymaps.o obj/save_and_load.o obj/utils.o
	$(CC) -o3 $(CFLAGS) $^ -o $@ 

target/$(DEBUG): obj/main.o obj/lb.o obj/arena.o obj/keymaps.o obj/save_and_load.o obj/utils.o
	$(CC) -g $(CFLAGS) $^ -o $@ 

# Object files
obj/main.o: src/main.c
	$(CC) -c $< -o $@

obj/lb.o: src/leaderboard.c
	$(CC) -c $< -o $@

obj/keymaps.o: src/keymaps.c
	$(CC) -c $< -o $@

obj/save_and_load.o: src/save_and_load.c
	$(CC) -c $< -o $@

obj/utils.o: src/utils.c
	$(CC) -c $< -o $@

obj/arena.o: lib/arena.c
	$(CC) -c $< -o $@

# Ensure directories exist
obj:
	@echo "Creating directory ./obj"
	mkdir -p ./obj

target:
	@echo "Creating directory ./target"
	mkdir -p ./target

game_files:
	@echo "creating directory ./game_files"
	mkdir -p ./game_files

# Clean target
clean:
	rm -rf obj target/$(DEBUG)
