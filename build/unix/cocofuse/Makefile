BINARY = cocofuse

vpath %.c ../../../$(BINARY)

CFLAGS	+= -I../../../include -Wall
LDFLAGS	+= -L../libtoolshed -L../libcoco -L../libnative -L../libdecb -L../libmisc -L../librbf -L../libsys -ltoolshed -lcoco -lnative -ldecb -lrbf -lmisc -lsys -lm -lfuse

$(BINARY):	$(BINARY).o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm *.o $(BINARY)
