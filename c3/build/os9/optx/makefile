include ../rules.mak

vpath %.c ../../../optexp
vpath %.h ../../../optexp
INCS	= -v=../../../optexp -v=../../../include

BINARY	= c3optx
OBJS	= inst.r labels.r main.r misc.r optim.r parse.r actions.r

$(BINARY):	$(OBJS)
	$(OS9LINK)

clean:
	rm -f $(BINARY) *.r

install: $(BINARY)
	$(os9install)
