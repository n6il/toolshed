include $(NITROS9DIR)/rules.mak

DBG			= lib.c/dbg.l
CLIB		= lib.c/cstuff.l
MAINLIB		= lib.a/main.l
FLOATLIB	= fmath/cfloats.l
TRANSLIB	= tmath/trans.l
MATHLIB		= imath/math.l
SYSLIB		= sys.a/syslib.l
CGFXLIB		= cgfx/cgfx.l
LIBS		= clibt.l clib.l sys_6809l1.l sys_6809l2.l sys_6309l2.l cgfx.l

lib6809l1: $(LIBS)
	-$(RM) sys.l
	$(SOFTLINK) sys_6809l1.l sys.l
	
lib6809l2: $(LIBS)
	-$(RM) sys.l
	$(SOFTLINK) sys_6809l2.l sys.l
	
lib6309l2: $(LIBS)
	-$(RM) sys.l
	$(SOFTLINK) sys_6309l2.l sys.l
	
$(CLIB):  lib.c/makefile lib.c/*.c
	$(CD) lib.c; make

$(DBG): lib.c/*.c lib.c/makefile
	$(CD) lib.c; make

$(MAINLIB): lib.a/*.a lib.a/*.c lib.a/makefile
	$(CD) lib.a; make

$(FLOATLIB): fmath/*.a fmath/makefile
	$(CD) fmath; make

$(TRANSLIB): tmath/*.a tmath/makefile
	$(CD) tmath; make

$(MATHLIB): imath/*.a imath/makefile
	$(CD) imath; make

$(SYSLIB): sys.a/*.a sys.a/makefile
	$(CD) sys.a; make

$(CGFXLIB): cgfx/*.a cgfx/*.c cgfx/makefile
	$(CD) cgfx; make

clib.l: $(CLIB) $(MAINLIB) $(FLOATLIB) $(MATHLIB) $(SYSLIB)
	-$(RM) clib.l
	$(MERGE) $^ >clib.l

clibt.l: $(CLIB) $(MAINLIB) $(TRANSLIB) $(MATHLIB) $(SYSLIB)
	-$(RM) clibt.l
	$(MERGE) $^ >clibt.l

cgfx.l: $(CGFXLIB)
	-$(RM) cgfx.l
	$(MERGE) $< >cgfx.l

sys_6809l1.l:	sys_6809l1.r
	-$(RM) $@
	$(MERGE) $< >$@

sys_6809l2.l:	sys_6809l2.r
	-$(RM) $@
	$(MERGE) $< >$@

sys_6309l2.l:	sys_6309l2.r
	-$(RM) $@
	$(MERGE) $< >$@

clean:
	$(CD) lib.c; make clean
	$(CD) lib.a; make clean
	$(CD) fmath; make clean
	$(CD) tmath; make clean
	$(CD) imath; make clean
	$(CD) sys.a; make clean
	$(CD) cgfx; make clean
	-$(RM) *.l *.r
