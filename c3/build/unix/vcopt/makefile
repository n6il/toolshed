include ../rules.mak

vpath %.c ../../../vcopt
vpath %.h ../../../vcopt

BINARY	= c3opt
OBJS	= copt2.o expr.o

$(BINARY):	$(OBJS)
	$(CC) $(OBJS) -o $@

clean:
	rm $(BINARY) *.o

install: $(BINARY)
	cp $(BINARY) $(HOME)/bin
