# Makefile for os9asm

vpath %.c ../../../os9asm

CFLAGS	+= -I../../../include
LDFLAGS	+= -L../libcoco -L../libnative -L../libdecb -L../librbf -L../libmisc -L../libsys -lcoco -lnative -ldecb -lrbf -lmisc -lsys

os9asm:		as.o crc.o eval.o output.o pseudo.o do9.o ffwd.o \
		print.o util.o env.o symtab.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm *.o os9asm
