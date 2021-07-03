CC = gcc
CFLAGS = -march=native -O2 -fstack-protector-strong -Wextra -Wall -I $(INCLUDE)

BIN ?= mdoc
DST = /usr/local/bin/$(BIN)

INCLUDE ?= include/
OBJDIR ?= build
SRCDIR ?= src


build:
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $(SRCDIR)/informative.c -o $(OBJDIR)/informative.o
	$(CC) $(CFLAGS) -c $(SRCDIR)/strman.c -o $(OBJDIR)/strman.o
	$(CC) $(CFLAGS) -c $(SRCDIR)/config.c -o $(OBJDIR)/config.o
	$(CC) $(CFLAGS) -c $(SRCDIR)/input.c -o $(OBJDIR)/input.o
	$(CC) $(CFLAGS) -c $(SRCDIR)/exec.c -o $(OBJDIR)/exec.o
	$(CC) $(CFLAGS) -c $(SRCDIR)/main.c -o $(OBJDIR)/main.o

.PHONY: clean install

install: 
	$(CC) $(CFLAGS) $(OBJDIR)/*.o -o $(DST)

clean:
	rm -rf $(OBJDIR)
