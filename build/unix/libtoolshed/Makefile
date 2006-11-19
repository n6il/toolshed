# Makefile for libtoolshed.a

vpath %.c ../../../libtoolshed

CFLAGS	+= -I../../../include -Wall

%.a:
	ar -r $@ $^
	ranlib $@

libtoolshed.a:	libtoolshed.o

clean:
	rm *.o *.a
