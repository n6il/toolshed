include ../rules.mak

vpath %.c ../../../exec
vpath %.h ../../../exec

CFLAGS		+=

BINARY	= c3
OBJS	= c3.o

$(BINARY):	$(OBJS)
	cc $(OBJS) -o $@

clean:
	rm $(BINARY) *.o

install: $(BINARY)
	cp $(BINARY) $(HOME)/bin
