# Makefile for decb

vpath %.c ../../../decb ../../../os9

CFLAGS	+= -I../../../include
LDFLAGS	+= -L../libtoolshed -L../libcoco -L../libnative -L../librbf -L../libdecb -L../libcecb -L../libmisc -L../libsys \
				-ltoolshed -lcoco -lnative -lrbf -ldecb -lcecb -lmisc -lsys -lm 

decb:	decb_main.o decbattr.o decbcopy.o decbdir.o decbdskini.o decbfree.o decbfstat.o \
	decbkill.o decblist.o decbrename.o os9dump.o decbhdbconv.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm *.o decb.exe
