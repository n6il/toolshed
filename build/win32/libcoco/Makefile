# Makefile for libcoco.a

vpath %.c ../../../libcoco

CFLAGS	+= -I../../../include

%.a:
	ar -r $@ $^
	ranlib $@

libcoco.a:	delete.o gs.o makdir.o open.o read.o readln.o rename.o seek.o ss.o write.o

clean:
	rm *.o *.a
