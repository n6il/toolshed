# Makefile for libnative.a

vpath %.c ../../../libnative

CFLAGS	+= -I../../../include

%.a:
	ar -r $@ $^
	ranlib $@

libnative.a:	libnativedelete.o libnativegs.o libnativemakdir.o \
libnativeopen.o libnativeread.o libnativereadln.o libnativerename.o \
libnativeseek.o libnativess.o libnativewrite.o

clean:
	rm -f *.o *.a
