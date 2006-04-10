# Makefile for libdecb.a

vpath %.c ../../../libdecb

CFLAGS	+= -g -I../../../include -Wall

%.a:
	ar -r $@ $^
	ranlib $@

libdecb.a:	gs.o kill.o open.o read.o rename.o seek.o ss.o read.o write.o tokenize.o

clean:
	rm *.o *.a
