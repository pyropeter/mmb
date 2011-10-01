# (adapted from
# http://www.cs.swarthmore.edu/~newhall/unixhelp/howto_makefiles.html)
#
# 'make depend'  uses makedepend to automatically generate dependencies 
#                (dependencies are added to end of Makefile)
# 'make'         alias for 'make otrtool && make doc'
# 'make doc'     gzips manpage
# 'make otrtool' build executable file 'otrtool'
# 'make clean'   removes all .o and executable files
#

SHELL = /bin/sh
.SUFFIXES:
.SUFFIXES: .c .o

DVERSION = v0.0.2
VERSION := $(shell git describe --long --dirty 2>/dev/null || echo "$(DVERSION)")

CC = gcc
CFLAGS = -Wall -g -DVERSION='"$(VERSION)"'
LIBS = -lGL -lGLU -lglut -lm

SRCS = src/render.c src/main.c
MAIN = mmb

OBJS = $(SRCS:.c=.o)

.PHONY: depend clean doc

all:    $(MAIN)
	@echo Done.

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) -o $(MAIN) $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS) $(MAIN)

depend: $(SRCS)
	makedepend -w70 $^

# DO NOT DELETE THIS LINE -- make depend needs it
