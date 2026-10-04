include ../rules.mak

vpath %.c ../../../comp
vpath %.h ../../../comp

# c3comp1 - front end (pass 1) of the two-pass SPLIT: parse C and emit the
# intermediate language (see comp/il.h).  Built from the shared comp sources
# with -DMKIL, which switches the codegen call sites (startfunc/endfunc/
# doreturn/get psect) over to IL emission.  Codegen objects are still linked
# for paths not yet converted to IL; they fall away as coverage grows.
CFLAGS += -DMKIL

BINARY	= c3comp1
OBJS	= build.o cmain.o declare.o emit.o get.o inits.o lex.o local.o \
	misc.o optim.o regscont.o stats.o treeutil.o ilwrite.o ilemit.o

$(BINARY):	$(OBJS)
	$(CC) $(OBJS) -o $@

clean:
	rm -f $(BINARY) *.o

install: $(BINARY)
	cp $(BINARY) $(HOME)/bin
