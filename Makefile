CC=gcc
BIN=start_max86150
CFLAGS=-I include -g3 -O2 -Wall -Wextra -lwiringPi -lpthread -lrt
CFILES=./src/main.c \
       ./src/filework.c \
       ./src/peripheral.c \
       ./src/timerwork.c

build_all:
	mkdir -p build
	time $(CC) -o ./build/$(BIN) $(CFILES) $(CFLAGS)

clean:
	rm -rf ./build/