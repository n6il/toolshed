# Makefile for libmisc.a
include ../rules.mak

vpath %.c ../../../libmisc

CFLAGS	+= -I../../../include -Wall

%.a:
	$(AR) -r $@ $^
	$(RANLIB) $@

libmisc.a:	libmiscendian.o libmisccococonv.o libmiscqueue.o libmiscutil.o

clean:
	$(RM) *.o *.a
