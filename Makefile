CC=cc
CFLAGS=-O2 -Wall -Wextra -std=c99 -D_XOPEN_SOURCE=500
LDFLAGS=-lncurses
OUTPUT=snake

prefix=/usr/local
bindir=$(prefix)/bin
man1dir=$(prefix)/share/man/man1
docdir=$(prefix)/share/doc/$(OUTPUT)

RM=rm -f
INSTALL=install
MKDIR=mkdir -p

OBJS=$(patsubst %.c,%.o,$(wildcard *.c))
DOCFILES=README CHANGES AUTHORS LICENSE


all: $(OUTPUT)

$(OUTPUT): $(OBJS)
	$(CC) -o $@ $(CFLAGS) $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $<

clean:
	$(RM) $(OUTPUT) $(OBJS) core core.*

distclean: clean

install:
	$(MKDIR) $(DESTDIR)$(bindir)
	$(MKDIR) $(DESTDIR)$(man1dir)
	$(MKDIR) $(DESTDIR)$(docdir)
	$(INSTALL) -m 755 $(OUTPUT) $(DESTDIR)$(bindir)
	$(INSTALL) -m 644 $(OUTPUT).1 $(DESTDIR)$(man1dir)
	$(INSTALL) -m 644 $(DOCFILES) $(DESTDIR)$(docdir)

.PHONY: clean distclean all

# dependencies:

snake.o: snake.c snake.h
