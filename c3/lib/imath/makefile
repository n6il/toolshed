include $(NITROS9DIR)/rules.mak

RFILES = abs.r minmax.r uminmax.r clmul.r cldiv.r claddsub.r clcompare.r \
	clbits.r clnegcompl.r clconvert.r clmove.r clincdec.r clshifts.r \
	clcommon.r ccmult.r ccmod.r ccdiv.r cshifts.r rpterr.r

math.l: $(RFILES) makefile
	cat $(RFILES) > math.l

clean:
	-$(RM) $(RFILES) math.l
