# Makefile for makewav

vpath %.c ../../../makewav
vpath %.h ../../../makewav

CFLAGS += -I../../../include

makewav:    makewav.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f *.o makewav.exe
