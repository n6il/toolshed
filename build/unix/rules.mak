# Project-Wide Rules
MERGE		= cat
SOFTLINK	= ln -s
RM		= rm
MAKE		= make
CFLAGS		= -Dunix -DUNIX -O3 -I. -g -I../../../include -Wall
ASM		= rma

%.l: %.r
	$(MERGE) $< > $@

%.r: %.a
	$(ASM) $< -o=$@
