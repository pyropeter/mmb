SHELL = /bin/sh
.SUFFIXES:
.SUFFIXES: .c .o

CODENAME = "Looks good as long as you don't move"
DVERSION = "v0.4.1"
VERSION := "$(shell git describe --tags --long --dirty 2>/dev/null \
		|| echo $(DVERSION))"

CC = gcc
CFLAGS = -Wall -g -pg
LIBS = -lGL -lGLU -lGLEW -lglut -lSOIL -lm -lrt
ALL_CFLAGS = $(CFLAGS)
ALL_CFLAGS += -DVERSION="$(subst ",\",$(subst \,\\,$(VERSION)))"
ALL_CFLAGS += -DCODENAME="$(subst ",\",$(subst \,\\,$(CODENAME)))"

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

all:    $(MAIN) gmcraft_terrain.png
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

doc/doxygen/cookie: doc/doxygen/Doxyfile $(SRCS) $(HEADERS)
	doxygen doc/doxygen/Doxyfile
	touch $@

gmcraft_terrain.png:
	wget http://eddie.pyropeter.eu/tmp/gmcraft_terrain.png

# DO NOT DELETE THIS LINE -- make depend needs it

src/worldrender.o: src/defs.h src/vector.h src/worldrender.h
src/worldrender.o: src/render.h src/world.h src/chunk.h src/block.h
src/worldrender.o: src/raytrace.h
src/render.o: src/render.h src/defs.h src/vector.h
src/chunksplit.o: src/defs.h src/chunksplit.h src/vector.h
src/chunksplit.o: src/world.h src/chunk.h src/block.h
src/world.o: src/defs.h src/vector.h src/world.h src/chunk.h
src/world.o: src/block.h src/chunkgen.h src/chunksplit.h src/bubble.h
src/simplex.o: src/cmwc.h src/simplex.h src/defs.h
src/defs.o: src/defs.h
src/chunkgen.o: src/defs.h src/chunkgen.h src/vector.h src/world.h
src/chunkgen.o: src/chunk.h src/block.h
src/generator.o: src/defs.h src/vector.h src/generator.h src/block.h
src/cmwc.o: src/cmwc.h
src/raytrace.o: src/raytrace.h src/vector.h src/world.h src/defs.h
src/raytrace.o: src/chunk.h src/block.h
src/main.o: src/defs.h src/vector.h src/render.h src/generator.h
src/main.o: src/block.h src/world.h src/chunk.h src/worldrender.h
src/main.o: src/raytrace.h
src/block.o: src/block.h src/vector.h
src/bubble.o: src/defs.h src/bubble.h src/vector.h src/world.h
src/bubble.o: src/chunk.h src/block.h
