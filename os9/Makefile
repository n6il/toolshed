# Makefile for os9 suite

include ../Makefile.rules

LDFLAGS	+= -L../lib
INCDIR	= ../include
LDLIBS	= -lrbf -los9sys -lmisc -lm
HEADERS	= $(INCDIR)/os9conv.h $(INCDIR)/os9module.h $(INCDIR)/os9path.h $(INCDIR)/os9sys.h \
	$(INCDIR)/os9types.h $(INCDIR)/util.h
CFLAGS	+= -I$(INCDIR)
OS9EXE	= os9$(EXE)
OBJS	= os9.o os9attr.o os9copy.o os9dir.o os9dsave.o os9gen.o \
	os9list.o os9id.o os9fstat.o \
	os9dump.o os9ident.o os9free.o os9cmp.o os9padrom.o \
	os9format.o os9del.o os9makdir.o os9modbust.o os9deldir.o \
	os9dcheck.o os9rename.o

all:	$(OS9EXE) $(HEADERS)

$(OS9EXE):	$(OBJS)

install:
	cp -f $(OS9EXE) $(ODIR)

package:
	cp -f $(OS9EXE) ../package

clean:
	rm -f $(OS9EXE) $(OBJS) core
