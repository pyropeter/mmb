SHELL = /bin/sh
.SUFFIXES:
.SUFFIXES: .c .o

DVERSION = v0.1
VERSION := $(shell git describe --tags --long --dirty 2>/dev/null \
		|| echo "$(DVERSION)")

CC = gcc
CFLAGS = -Wall -g -pg
ALL_CFLAGS = -DVERSION='"$(VERSION)"' $(CFLAGS)
LIBS = -lGL -lGLU -lglut -lm -lrt

# for stable releases
# * disable symbols?
# * disable profiling
# * disable assertions
#CFLAGS = -Wall -DNDEBUG=1

SRCS = src/render.c src/generator.c src/main.c src/world.c src/defs.c \
	src/chunkgen.c src/raytrace.c
MAIN = mmb

OBJS = $(SRCS:.c=.o)

.PHONY: depend clean

all:    $(MAIN)
	@echo Done.

$(MAIN): $(OBJS)
	$(CC) $(ALL_CFLAGS) -o $(MAIN) $(OBJS) $(LIBS)

.c.o:
	$(CC) $(ALL_CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS) $(MAIN)

depend: $(SRCS)
	makedepend -w70 -Y $^

# DO NOT DELETE THIS LINE -- make depend needs it

src/render.o: src/render.h src/defs.h src/vector.h
src/generator.o: src/defs.h src/vector.h src/generator.h
src/main.o: src/defs.h src/vector.h src/render.h src/generator.h
src/main.o: src/chunk.h src/world.h src/raytrace.h
src/world.o: src/defs.h src/vector.h src/chunk.h src/chunkgen.h
src/world.o: src/world.h
src/defs.o: src/defs.h
src/chunkgen.o: src/defs.h src/vector.h src/chunk.h src/world.h
src/chunkgen.o: src/chunkgen.h
src/raytrace.o: src/vector.h src/chunk.h src/defs.h src/world.h
src/raytrace.o: src/raytrace.h
