# Makefile for libmisc.a

vpath %.c ../../../libmisc

CFLAGS	+= -I../../../include

%.a:
	ar -r $@ $^
	ranlib $@

libmisc.a:	libmiscendian.o libmisccococonv.o libmiscqueue.o libmiscutil.o

clean:
	rm -f *.o *.a
