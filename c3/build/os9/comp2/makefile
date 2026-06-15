include ../rules.mak

vpath %.c ../../../comp
vpath %.h ../../../comp
INCS	= -v=../../../comp -v=../../../include

# c3comp2 - back end (pass 2) of the two-pass SPLIT, as an OS-9 module: read the
# intermediate language (comp/il.h) and generate 6809 assembly, reusing the
# existing back end (optim type-check, tranexp, codgen, local).  No parser, no
# symbol table - cmain2.c rebuilds postfix trees from the IL and replays the
# codegen.  Mirrors build/unix/comp2.
BINARY	= c3comp2
OBJS	= cmain2.r ilread.r codgen.r tranexp.r optim.r local.r bool.r longs.r \
	  regscont.r floats.r misc.r treeutil.r

$(BINARY):	$(OBJS)
	$(OS9LINK)

clean:
	rm -f $(BINARY) *.r

install: $(BINARY)
	$(os9install)
