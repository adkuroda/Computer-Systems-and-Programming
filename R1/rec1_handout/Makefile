all: r1

CFLAGS=-g -Og -Wall -Werror -Wno-error=unused-variable -std=gnu99
CC=gcc

r1: r1.c r1.h 
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f r1
