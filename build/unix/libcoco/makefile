# Makefile for libcoco.a

vpath %.c ../../../libcoco

CFLAGS	+= -g -I../../../include

%.a:
	ar -r $@ $^
	ranlib $@

libcoco.a:	gs.o delete.o open.o read.o rename.o seek.o ss.o read.o write.o

clean:
	rm *.o *.a
