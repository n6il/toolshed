# Makefile for rlink

vpath %.c ../../../rlink
vpath %.h ../../../rlink

CFLAGS += -I../../../include
LDFLAGS += -Xlinker --enable-stdcall-fixup

rlink:  decbout.o os9out.o rl_pass2.o rl_pass1b.o rl_pass1a.o rlink_main.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS) -lwsock32

clean:
	rm -f *.o rlink.exe
