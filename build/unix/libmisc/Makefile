# Makefile for libmisc.a
include ../rules.mak

vpath %.c ../../../libmisc

CFLAGS	+= -I../../../include -Wall

%.a:
	ar -r $@ $^
	ranlib $@

libmisc.a:	libmiscendian.o libmisccococonv.o libmiscqueue.o libmiscutil.o

clean:
	rm *.o *.a
