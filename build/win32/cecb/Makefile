# Makefile for cecb

vpath %.c ../../../cecb ../../../decb ../../../os9
vpath %.h ../../../include

CFLAGS	+= -I../../../include
LDFLAGS	+= -L../libtoolshed -L../libcoco -L../libnative -L../librbf -L../libdecb -L../libcecb -L../libmisc -L../libsys \
			-ltoolshed -lcoco -lnative -lrbf -ldecb -lcecb -lmisc -lsys -lm 

cecb:	cecbfstat.o cecbdir.o cecbcopy.o cecbbulkerase.o cecb_main.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm *.o cecb.exe
