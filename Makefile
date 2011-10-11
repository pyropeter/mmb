SHELL = /bin/sh
.SUFFIXES:
.SUFFIXES: .c .o

DVERSION = v0.0.2
VERSION := $(shell git describe --long --dirty 2>/dev/null || echo "$(DVERSION)")

CC = gcc
CFLAGS = -Wall -g -DVERSION='"$(VERSION)"'
LIBS = -lGL -lGLU -lglut -lm

SRCS = src/render.c src/generator.c src/main.c src/chunk.c
MAIN = mmb

OBJS = $(SRCS:.c=.o)

.PHONY: depend clean

all:    $(MAIN)
	@echo Done.

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) -o $(MAIN) $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS) $(MAIN)

depend: $(SRCS)
	makedepend -w70 -Y $^

# DO NOT DELETE THIS LINE -- make depend needs it

src/render.o: src/render.h src/defs.h
src/generator.o: src/generator.h src/defs.h
src/main.o: src/defs.h src/render.h src/generator.h src/chunk.h
