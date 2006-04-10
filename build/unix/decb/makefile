# Makefile for decb

vpath %.c ../../../decb ../../../os9

CFLAGS	+= -g -I../../../include -Wall
LDFLAGS	+= -g -L../libcoco -L../libnative -L../librbf -L../libdecb -L../libmisc -L../libsys -lcoco -lnative -lrbf -ldecb -lmisc -lsys -lm

decb:	decb.o decbattr.o decbcopy.o decbdir.o decbdskini.o decbfree.o decbfstat.o \
	decbhdbconv.o decbkill.o decblist.o decbrename.o os9dump.o ../libdecb/libdecb.a
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm *.o decb
