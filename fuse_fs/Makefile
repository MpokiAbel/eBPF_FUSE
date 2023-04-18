CC=gcc
CFLAGS=-Wall $(shell pkg-config fuse3 --cflags)
LIBS=$(shell pkg-config fuse3 --libs)

PROGRAMS = stackfs client server

.PHONY: all $(PROGRAMS)

all: $(PROGRAMS)

$(PROGRAMS):
	$(CC) $(CFLAGS) $@.c $(LIBS) -o $@

clean:
	rm -f $(PROGRAMS)

