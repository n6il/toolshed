# Makefile for cecb
include ../rules.mak

vpath %.c ../../../cecb ../../../os9

CFLAGS	+= -g -I../../../include -Wall
LDFLAGS	+= -g -L../libcoco -L../libnative -L../libcecb -L../librbf -L../libdecb -L../libmisc -L../libsys -lcoco -ldecb -lnative -lrbf -lcecb -lmisc -lsys -lm 

cecb:	cecbbulkerase.o cecbdir.o cecbfstat.o cecb_main.o cecbcopy.o ../os9/os9dump.o ../decb/decblist.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) *.o cecb cecb.exe
