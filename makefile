CC = gcc
CFLAGS = -std=c23 -Wall -Werror -lncurses
DEBUG = debug
RELEASE = uni-void

all: target target/$(DEBUG)

release: target/$(RELEASE)

# Run target
run: target target/$(DEBUG)
	./target/$(DEBUG)


target/$(RELEASE): src/main.c
	$(CC) -o3 $(CFLAGS) $^ -o $@

target/$(DEBUG): src/main.c
	$(CC) -g $^ -o $@ $(CFLAGS)

# directory check
target:
	@echo "Creating directory ./target"
	mkdir -p ./target

