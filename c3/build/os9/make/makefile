include ../rules.mak

vpath %.c ../../../make
vpath %.h ../../../make
INCS	= -v=../../../make -v=../../../include

BINARY	= make
OBJS	= check.r files.r input.r macro.r main.r make.r reader.r rules.r

$(BINARY):	$(OBJS)
	$(OS9LINK)

clean:
	rm -f $(BINARY) *.r

install: $(BINARY)
	$(os9install)
