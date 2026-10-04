include ../rules.mak

vpath %.c ../../../exec
vpath %.h ../../../exec

# Compiled-in default for $C3BASE (where the driver finds defs/ when the env
# var is unset).  The top-level ToolShed install passes its PREFIX-derived
# value; the fallback is the standard /usr/local prefix.
PREFIX		?= /usr/local
C3BASE_DEFAULT	?= $(PREFIX)/share/toolshed
CFLAGS		+= -DC3BASE_DEFAULT='"$(C3BASE_DEFAULT)"'

BINARY	= c3
OBJS	= c3.o

$(BINARY):	$(OBJS)
	$(CC) $(OBJS) -o $@

clean:
	rm -f $(BINARY) *.o

install: $(BINARY)
	cp $(BINARY) $(HOME)/bin
