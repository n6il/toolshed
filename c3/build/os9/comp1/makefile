include ../rules.mak

vpath %.c ../../../comp
vpath %.h ../../../comp
INCS	= -v=../../../comp -v=../../../include

# c3comp1 - front end (pass 1) of the two-pass SPLIT, as an OS-9 module: parse C
# and emit the intermediate language (see comp/il.h).  -DMKIL (added to the
# preprocessor flags so c3prep sees it) switches the codegen call sites over to
# IL emission.  Codegen objects are still linked for paths not yet converted to
# IL; they fall away as coverage grows.  Mirrors build/unix/comp1.
CPPFLAGS += -DMKIL

BINARY	= c3comp1
OBJS	= bool.r build.r cmain.r codgen.r declare.r floats.r get.r inits.r lex.r \
	  local.r longs.r misc.r optim.r regscont.r stats.r tranexp.r treeutil.r \
	  ilwrite.r ilemit.r

$(BINARY):	$(OBJS)
	$(OS9LINK)

clean:
	rm -f $(BINARY) *.r

install: $(BINARY)
	$(os9install)
