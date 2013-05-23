# maths.l makefile

RDIR = rels

FILES = cfloats.r clmul.r cldiv.r claddsub.r clcompare.r clbits.r\
	clnegcompl.r clconvert.r clmove.r clincdec.r clshifts.r\
	clcommon.r ccmult.r ccmod.r ccdiv.r cshifts.r rpterr.r

maths.l: $(FILES)
	-del maths.l
	(chd $(RDIR) merge $(FILES)) >/r0/maths.l
	copy /r0/maths.l maths.l
	del /r0/maths.l

