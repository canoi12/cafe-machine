CC = gcc
SOURCE = main.c coffee.c
CFLAGS = -std=c99
LFLAGS = -Wall
OUT = vm

build:
	$(CC) $(SOURCE) -o $(OUT) $(CFLAGS) $(LFLAGS)

clean:
	rm -rf vm