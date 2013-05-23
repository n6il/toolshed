include $(NITROS9DIR)/rules.mak

CFLAGS = -s -q -o -v=../../defs

########################################################
# Makefile for CGFX library version 7
# by Mike Sweet 9/11/90
#
OBJS = getwind.r setwind.r cbuffer.r polyline.r polyfill.r polyfunc.r \
       dialog.r mousekey.r settype.r sinecosine.r\
       button.r tandy.r object.r mvmenu.r mvmenuxy.r mvfname.r \
       getstr.r fname.r menu.r menuxy.r shadow.r text.r getput.r play.r \
       window.r config.r font.r radio.r draw.r status.r mouse.r \
       alarm.r mv_window.r keysense.r sound.r crw.r mousexy.r \
       getenv.r movemem.r drawfunc.r nobuffer.r
       
cgfx: $(OBJS) malloc.r makefile
	cat $(OBJS) > cgfx.l

clean:
	rm -f cgfx.l *.r
