CC=gcc
CFLAGS=-Wall $(shell pkg-config fuse --cflags)
LIBS=$(shell pkg-config fuse --libs)

PROGRAMS = hello stackfs

.PHONY: all $(PROGRAMS)

all: $(PROGRAMS)

$(PROGRAMS):
	$(CC) $(CFLAGS) $@.c $(LIBS) -o $@

clean:
	rm -f $(PROGRAMS)

