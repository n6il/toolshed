# Makefile for decb

vpath %.c ../../../decb

CFLAGS	+= -I../../../include
LDFLAGS	+= -L../libdecb -L../libmisc -ldecb -lmisc -lm

decb:	decb.o decbcopy.o decbdir.o decbformat.o decbfree.o decbfstat.o \
	decbkill.o decblist.o decbrename.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm *.o decb
