# Makefile for libnative.a

vpath %.c ../../../libnative

CFLAGS	+= -g -I../../../include

%.a:
	ar -r $@ $^
	ranlib $@

libnative.a:	gs.o delete.o open.o read.o rename.o seek.o ss.o read.o write.o

clean:
	rm *.o *.a
