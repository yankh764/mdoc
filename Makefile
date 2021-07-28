CC = gcc
CFLAGS = -O2 -pipe -march=native -fstack-protector-strong -Wextra -Wall -I$(INCLUDE)

BIN ?= mdoc
DST ?= /usr/local/bin/$(BIN)

INCLUDE ?= include/
OBJDIR ?= ./build
SRCDIR ?= ./src

SRCS := $(shell find $(SRCDIR) -iname "*.c")
OBJS := $(SRCS:%=$(OBJDIR)/%.o)


generate_bin: $(OBJS)
	$(CC) $(OBJS) -o ./$(BIN) $(LDFLAGS)


$(OBJDIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@


.PHONY: clean install uninstall


clean:
	$(RM) -r $(OBJDIR)
	$(RM) ./$(BIN)

install:
	install ./$(BIN) $(DST)

uninstall: clean
	$(RM) $(DST)
