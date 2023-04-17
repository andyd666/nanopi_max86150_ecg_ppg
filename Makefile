CC=gcc
BIN=start_max86150
CFLAGS=-I include -g3 -O2 -Wall -Wextra -lwiringPi -lpthread
CFILES=./src/main.c \
       ./src/filework.c \
       ./src/peripheral.c

build_all:
	mkdir -p build
	$(CC) -o ./build/$(BIN) $(CFILES) $(CFLAGS)

clean:
	rm -rf ./build/