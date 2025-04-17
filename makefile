CC = clang
CFLAGS = -std=c23 -Wall -Werror -lncurses
DEBUG = debug
RELEASE = uni-void

ifeq ($(shell pkg-config --exists ncurses && echo yes),)
    $(error ncurses library not found. Please install it before compiling.)
endif

all: game_files target target/$(DEBUG)

release: game_files target target/$(RELEASE)

# Run target
run: target game_files target/$(DEBUG)
	./target/$(DEBUG)


target/$(RELEASE): src/*
	$(CC) -o3 -std=c23 -lncurses src/main.c -o $@

target/$(DEBUG): src/*
	$(CC) -g $(CFLAGS) src/main.c -o $@ 

# directory check
target:
	@echo "Creating directory ./target"
	mkdir -p ./target

game_files:
	@echo "Creating directory ./game_files"
	mkdir -p ./game_files

