CC=cc
CFLAGS=-O2 -Wall -Wextra
LDFLAGS=-lncurses
OUTPUT=snake
OBJS=$(patsubst %.c,%.o,$(wildcard *.c))

all: $(OUTPUT)

$(OUTPUT): $(OBJS)
	$(CC) -o $@ $(CFLAGS) $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $<

clean:
	$(RM) $(OUTPUT) $(OBJS) core core.*

distclean: clean

.PHONY: clean distclean all
