# Makefile for libsys.a

vpath %.c ../../../libsys

CFLAGS	+= -I../../../include -Wall

%.a:
	ar -r $@ $^
	ranlib $@

libsys.a:	crc.o prsnam.o

clean:
	rm *.o *.a
