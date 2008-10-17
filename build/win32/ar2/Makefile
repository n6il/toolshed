# Makefile for decb

vpath %.c ../../../ar2

CFLAGS	+= -I../../../include -DSYSV
LDFLAGS	+= -L../libtoolshed -L../libcoco -L../libnative -L../librbf -L../libdecb -L../libmisc -L../libsys -ltoolshed -lcoco -lnative -lrbf -ldecb -lmisc -lsys -lm 

ar2:	ar.o arsup.o lz1.o o2u.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm *.o ar2.exe
