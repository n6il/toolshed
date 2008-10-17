# Makefile for makewav

vpath %.c ../../../makewav
vpath %.h ../../../makewav

CFLAGS += -I../../../include

makewav:	makewav.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm *.o makewav.exe
