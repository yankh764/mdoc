CC = gcc
CFLAGS = -O2 -pipe -march=native -fstack-protector-strong -Wextra \
		 -Wall -Wundef -Wformat=2 -Wstrict-overflow=5 -I$(INCLUDE)

DST_DIR = /usr/local/bin

BIN ?= mdoc
INCLUDE ?= include/
OBJDIR ?= build
SRCDIR ?= src

SRCS := $(shell find $(SRCDIR) -iname "*.c")
OBJS := $(SRCS:%=$(OBJDIR)/%.o)



$(BIN): $(OBJS)
	$(CC) $(OBJS) -o ./$@ $(LDFLAGS)

$(OBJDIR)/%.c.o: %.c 
	mkdir -p $(dir $@) 
	$(CC) $(CFLAGS) -c $< -o $@



.PHONY: clean install uninstall all



#For the users who like to type 'make all'
all: $(BIN)

install:
	install ./$(BIN) $(DST_DIR)/$(BIN)

clean:
	$(RM) -r $(OBJDIR)
	$(RM) ./$(BIN)

uninstall: clean
	$(RM) $(DST_DIR)/$(BIN)
