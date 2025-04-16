CC = gcc
CFLAGS = -std=c23 -Wall -Werror -lncurses
DEBUG = debug
RELEASE = uni-void

all: target target/$(DEBUG)

release: target/$(RELEASE)

# Run target
run: target target/$(DEBUG)
	./target/$(DEBUG)


target/$(RELEASE): src/*
	$(CC) -o3 -std=c23 -lncurses src/main.c -o $@

target/$(DEBUG): src/*
	$(CC) -g $(CFLAGS) src/main.c -o $@ 

# directory check
target:
	@echo "Creating directory ./target"
	mkdir -p ./target

