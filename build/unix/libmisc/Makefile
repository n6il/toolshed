# Makefile for libmisc.a

vpath %.c ../../../libmisc

CFLAGS	+= -I../../../include

%.a:
	ar -r $@ $^
	ranlib $@

libmisc.a:	endian.o cococonv.o os9diskfuncs.o queue.o util.o

clean:
	rm *.o *.a
