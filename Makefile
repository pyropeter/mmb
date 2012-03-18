SHELL = /bin/sh
.SUFFIXES:
.SUFFIXES: .c .o

CODENAME = "White, Removable Cubes"
DVERSION = v0.2
VERSION := $(shell git describe --tags --long --dirty 2>/dev/null \
		|| echo "$(DVERSION)")

CC = gcc
CFLAGS = -Wall -g -pg
ALL_CFLAGS = -DVERSION='"$(VERSION)"' -DCODENAME='$(CODENAME)' $(CFLAGS)
LIBS = -lGL -lGLU -lGLEW -lglut -lm -lrt

# for stable releases
# * disable symbols?
# * disable profiling
# * disable assertions
#CFLAGS = -Wall -DNDEBUG=1

HEADERS = $(shell find src/ -name '*.h')
SRCS = $(shell find src/ -name '*.c')
OBJS = $(SRCS:.c=.o)

MAIN = mmb

.PHONY: depend clean doc

all:    $(MAIN)
	@echo Done.

$(MAIN): $(OBJS)
	$(CC) $(ALL_CFLAGS) -o $(MAIN) $(OBJS) $(LIBS)

.c.o:
	$(CC) $(ALL_CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS) $(MAIN) doc/graph/graph.dot doc/graph/graph.png \
		doc/doxygen/cookie

depend: $(SRCS)
	makedepend -w70 -Y $^

doc: doc/graph/graph.png doc/doxygen/cookie
	@echo Documentation generated.

doc/graph/graph.dot: $(SRCS) $(HEADERS)
	./doc/graph/create $(SRCS) $(HEADERS) > $@

doc/graph/graph.png: doc/graph/graph.dot
	dot -Tpng doc/graph/graph.dot > $@

doc/doxygen/cookie: $(SRCS) $(HEADERS)
	doxygen doc/doxygen/Doxyfile
	touch $@

# DO NOT DELETE THIS LINE -- make depend needs it

src/chunksplit.o: src/defs.h src/chunksplit.h src/vector.h
src/chunksplit.o: src/world.h src/chunk.h src/block.h
src/chunkgen.o: src/defs.h src/chunkgen.h src/vector.h src/world.h
src/chunkgen.o: src/chunk.h src/block.h
src/generator.o: src/defs.h src/vector.h src/generator.h src/block.h
src/worldrender.o: src/defs.h src/vector.h src/worldrender.h
src/worldrender.o: src/render.h src/world.h src/chunk.h src/block.h
src/worldrender.o: src/raytrace.h
src/defs.o: src/defs.h
src/main.o: src/defs.h src/vector.h src/render.h src/generator.h
src/main.o: src/block.h src/world.h src/chunk.h src/worldrender.h
src/main.o: src/raytrace.h
src/world.o: src/defs.h src/vector.h src/world.h src/chunk.h
src/world.o: src/block.h src/chunkgen.h src/chunksplit.h
src/render.o: src/render.h src/defs.h src/vector.h
src/raytrace.o: src/raytrace.h src/vector.h src/world.h src/defs.h
src/raytrace.o: src/chunk.h src/block.h
