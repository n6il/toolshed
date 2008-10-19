# Makefile for libtoolshed.a

vpath %.c ../../../libtoolshed

CFLAGS	+= -I../../../include

%.a:
	ar -r $@ $^
	ranlib $@

libtoolshed.a:	toolshed.o

clean:
	rm -f *.o *.a
