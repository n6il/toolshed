# Makefile for libtoolshed.a
include ../rules.mak

vpath %.c ../../../libtoolshed

CFLAGS	+= -I../../../include -Wall

%.a:
	ar -r lib$@ $^
	ranlib lib$@

toolshed.a:	toolshed.o

clean:
	rm *.o *.a
