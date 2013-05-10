# Makefile for libdecb.a

vpath %.c ../../../libdecb

CFLAGS	+= -I../../../include

%.a:
	ar -r $@ $^
	ranlib $@

libdecb.a:	libdecbgs.o libdecbkill.o libdecbopen.o libdecbread.o \
libdecbrename.o libdecbseek.o libdecbss.o libdecbwrite.o libdecbtokenize.o \
libdecbbinconcat.o libdecbsrec.o

clean:
	rm -f *.o *.a
