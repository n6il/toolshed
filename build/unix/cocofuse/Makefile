include ../rules.mak

BINARY = cocofuse

vpath %.c ../../../$(BINARY)

CFLAGS	+= -I../../../include -Wall
LDFLAGS	+= -L../libtoolshed -L../libcoco -L../libnative -L../libcecb -L../libdecb -L../libmisc -L../librbf -L../libsys -ltoolshed -lcoco -lnative -lcecb -ldecb -lrbf -lmisc -lsys -lm -lfuse

$(BINARY):	$(BINARY).o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm *.o $(BINARY)
