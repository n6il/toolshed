# Makefile for libdecb.a

vpath %.c ../../../libdecb

CFLAGS	+= -I../../../include

%.a:
	ar -r $@ $^
	ranlib $@

libdecb.a:	gs.o kill.o open.o read.o rename.o seek.o ss.o write.o

clean:
	rm *.o *.a
