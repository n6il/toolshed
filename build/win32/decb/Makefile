# Makefile for decb

vpath %.c ../../../decb ../../../os9
vpath %.h ../../../include

CFLAGS	+= -I../../../include
LDFLAGS	+= -L../libtoolshed -L../libcoco -L../libnative -L../librbf -L../libdecb -L../libcecb -L../libmisc -L../libsys \
				-ltoolshed -lcoco -lnative -lrbf -ldecb -lcecb -lmisc -lsys

decb:	decb_main.o decbattr.o decbcopy.o decbdir.o decbdskini.o decbfree.o decbfstat.o \
	decbkill.o decblist.o decbrename.o os9dump.o decbhdbconv.o decbdsave.o os9dsave.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f *.o decb.exe
