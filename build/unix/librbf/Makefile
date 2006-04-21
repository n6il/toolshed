# Makefile for librbf.a

vpath %.c ../../../librbf

CFLAGS	+= -I../../../include -Wall -g

%.a:
	ar -r $@ $^
	ranlib $@

librbf.a:	bitmap.o makdir.o read.o rename.o ss.o delete.o \
		gs.o open.o readln.o seek.o write.o

clean:
	rm *.o *.a
