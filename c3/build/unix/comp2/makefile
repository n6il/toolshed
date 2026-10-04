include ../rules.mak

vpath %.c ../../../comp
vpath %.h ../../../comp

# c3comp2 - back end (pass 2) of the two-pass SPLIT: read the intermediate
# language (comp/il.h) and generate 6809 assembly, reusing the existing back
# end (optim type-check, tranexp, codgen, local).  No parser, no symbol table -
# cmain2.c rebuilds postfix trees from the IL and replays the codegen.
BINARY	= c3comp2
OBJS	= cmain2.o cmain2b.o cmain2c.o ilread.o codgen.o emit.o tranexp.o optim.o local.o bool.o longs.o \
	regscont.o floats.o misc.o treeutil.o

$(BINARY):	$(OBJS)
	$(CC) $(OBJS) -o $@

clean:
	rm -f $(BINARY) *.o

install: $(BINARY)
	cp $(BINARY) $(HOME)/bin
