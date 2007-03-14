# Makefile for libtoolshed.a

vpath %.c ../../../libtoolshed

CFLAGS	+= -I../../../include -Wall

%.a:
	ar -r lib$@ $^
	ranlib lib$@

toolshed.a:	toolshed.o

clean:
	rm *.o *.a
