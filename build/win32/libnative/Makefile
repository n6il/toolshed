# Makefile for libnative.a

vpath %.c ../../../libnative

CFLAGS	+= -I../../../include

%.a:
	ar -r $@ $^
	ranlib $@

libnative.a:	delete.o gs.o makdir.o open.o read.o readln.o rename.o seek.o ss.o write.o

clean:
	rm *.o *.a
