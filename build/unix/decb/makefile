# Makefile for decb

vpath %.c ../../../decb

CFLAGS	+= -g -I../../../include
LDFLAGS	+= -g -L../libcoco -L../libnative -L../librbf -L../libdecb -L../libmisc -L../libsys -lcoco -lnative -lrbf -ldecb -lmisc -lsys -lm

decb:	decb.o decbattr.o decbcopy.o decbdir.o decbdskini.o decbfree.o decbfstat.o \
	decbkill.o decblist.o decbrename.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm *.o decb
