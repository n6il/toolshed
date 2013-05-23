include $(NITROS9DIR)/rules.mak

RFILES = trig.r hyp.r log.r trnser.r dsqrt.r dinc.r \
	dtype.r dmul.r ddiv.r daddsub.r dser.r dcmpr.r

trans.l: $(RFILES) makefile
	cat $(RFILES) > trans.l

clean:
	-$(RM) $(RFILES) trans.l
