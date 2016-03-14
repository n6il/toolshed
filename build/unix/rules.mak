# Project-Wide Rules

VERSION = 2.1

MERGE		= cat
SOFTLINK	= ln -s
RM		= rm -f
INSTALL		= install
MAKE		= make
CFLAGS		= -Dunix -DUNIX -O3 -I. -g -I../../../include -Wall -DTOOLSHED_VERSION=\"$(VERSION)\"
ASM		= rma
AR		= $(CROSS)ar
RANLIB		= $(CROSS)ranlib
CC		= $(CROSS)cc

%.l: %.r
	$(MERGE) $< > $@

%.r: %.a
	$(ASM) $< -o=$@
