# Makefile for rlink

vpath %.c ../../../rlink
vpath %.h ../../../rlink

CFLAGS += -I../../../include

rlink:	decbout.o os9out.o rl_pass2.o rl_pass1b.o rl_pass1a.o rlink_main.o
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm rlink.exe *.o
