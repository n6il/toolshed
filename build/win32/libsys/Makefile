# Makefile for libsys.a

vpath %.c ../../../libsys

CFLAGS	+= -I../../../include

%.a:
	ar -r $@ $^
	ranlib $@

libsys.a:	crc.o prsnam.o

clean:
	rm *.o *.a
