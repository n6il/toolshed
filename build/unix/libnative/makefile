# Makefile for libnative.a

vpath %.c ../../../libnative

CFLAGS	+= -g -I../../../include -Wall

%.a:
	ar -r $@ $^
	ranlib $@

libnative.a:	nativegs.o nativedelete.o nativeopen.o nativeread.o nativerename.o nativeseek.o nativess.o nativeread.o nativewrite.o nativereadln.o
clean:
	rm *.o *.a
