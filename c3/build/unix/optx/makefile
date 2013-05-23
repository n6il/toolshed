include ../rules.mak

vpath %.c ../../../optexp
vpath %.h ../../../optexp

BINARY	= c3optx
OBJS	= inst.o labels.o main.o misc.o optim.o parse.o symtab.o 

$(BINARY):	$(OBJS) op.h actions.h
	$(CC) $(OBJS) -o $@

clean:
	rm $(BINARY) *.o

install: $(BINARY)
	cp $(BINARY) $(HOME)/bin
