include $(NITROS9DIR)/rules.mak

CFLAGS = -s -q -o -v=../../defs

########################################################
# Makefile for CGFX library version 7
# by Mike Sweet 9/11/90
#
OBJS = crtbynam.r crtbypth.r termcap.r tparam.r termcap.r
       
termcap: $(OBJS) makefile
	cat $(OBJS) > termcap.l

clean:
	rm -f termcap.l *.r
