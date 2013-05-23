include $(NITROS9DIR)/rules.mak

CFLAGS = -s -q -o -v=../../defs
RFILES = prof.r pwent.r getopt.r defdrive.r popen.r adump.r
DFILES = dbglib.r getsp.r

all:	cstuff.l dbg.l

cstuff.l: $(RFILES) makefile
	cat $(RFILES) > cstuff.l

dbg.l: $(DFILES) makefile
	cat dbglib.r getsp.r > dbg.l

clean:
	-$(RM) $(RFILES) $(DFILES) cstuff.l dbg.l
