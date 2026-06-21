include ../rules.mak

vpath %.c ../../../comp
vpath %.h ../../../comp
INCS	= -v=../../../comp -v=../../../include

BINARY	= c3comp
OBJS	= bool.r build.r cmain.r codgen.r declare.r emit.r floats.r get.r inits.r lex.r \
	  local.r longs.r misc.r optim.r regscont.r stats.r tranexp.r treeutil.r

$(BINARY):	$(OBJS)
	$(OS9LINK)

clean:
	rm -f $(BINARY) *.r

install: $(BINARY)
	$(os9install)
