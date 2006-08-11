# Makefile for libnative.a

vpath %.c ../../../libnative

CFLAGS	+= -g -I../../../include -Wall

%.a:
	ar -r $@ $^
	ranlib $@

libnative.a:	libnativegs.o libnativedelete.o libnativeopen.o libnativeread.o libnativerename.o libnativeseek.o libnativess.o libnativeread.o libnativewrite.o libnativereadln.o
clean:
	rm *.o *.a
