include ../rules.mak

vpath %.c ../../../exec
vpath %.h ../../../exec

CFLAGS		+=

BINARY	= c3
OBJS	= c3.o

$(BINARY):	$(OBJS)
	$(CC) $(OBJS) -o $@

clean:
	rm -f $(BINARY) *.o

install: $(BINARY)
	cp $(BINARY) $(HOME)/bin
