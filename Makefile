CC = gcc
CFLAGS = -march=native -O2 -fstack-protector-strong -Wextra -Wall -I $(INCLUDE)

BIN ?= mdoc

INCLUDE ?= include/
OBJDIR ?= build
SRCDIR ?= src


build:
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $(SRCDIR)/informative.c -o $(OBJDIR)/informative.o
	$(CC) $(CFLAGS) -c $(SRCDIR)/strman.c -o $(OBJDIR)/strman.o
	$(CC) $(CFLAGS) -c $(SRCDIR)/config.c -o $(OBJDIR)/config.o
	$(CC) $(CFLAGS) -c $(SRCDIR)/types.c -o $(OBJDIR)/types.o
	$(CC) $(CFLAGS) -c $(SRCDIR)/input.c -o $(OBJDIR)/input.o
	$(CC) $(CFLAGS) -c $(SRCDIR)/exec.c -o $(OBJDIR)/exec.o


.PHONY: clean test

test: 
	$(CC) $(CFLAGS) $(SRCDIR)/*.c test.c

clean:
	rm -rf $(OBJDIR)
