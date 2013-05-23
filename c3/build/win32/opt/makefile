include ../rules.mak

vpath %.c ../../../opt
vpath %.h ../../../opt

BINARY	= c3opt
#OBJS	= inst.o labels.o main.o misc.o optim.o parse.o symtab.o 
OBJS	= labels.o main.o misc.o optim.o parse.o symtab.o 

$(BINARY):	$(OBJS)
	cc $(OBJS) -o $@

clean:
	rm $(BINARY) *.o

install: $(BINARY)
	cp $(BINARY) $(HOME)/bin
