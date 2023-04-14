CC=gcc
OBJ=start_max86150
CFLAGS=-I include -g3 -O2 -Wall -Wextra -lwiringPi
CFILES=main.c

build_all:
	mkdir -p build
	$(CC) $(CFLAGS) -o ./build/$(OBJ) ./src/$(CFILES)

clean:
	rm -rf ./build/