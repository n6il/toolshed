include ../rules.mak

vpath %.c ../../../rlink
vpath %.h ../../../rlink

# CFLAGS += -DDEBUG

BINARY	= rlink
OBJS	= rlink.o rl_pass1a.o rl_pass1b.o rl_pass2.o os9out.o decbout.o

$(BINARY):	$(OBJS)
	$(CC) $(OBJS) -o $@

$(OBJS):	rlink.h out.h ../include/rof.h

clean:
	rm $(BINARY) *.o

install: $(BINARY)
	cp $(BINARY) $(HOME)/bin
