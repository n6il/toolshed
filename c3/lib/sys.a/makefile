include $(NITROS9DIR)/rules.mak

RFILES = abort.r signal.r chown.r chmod.r stat.r access.r create.r \
	open.r read.r write.r lseek.r misc.r mod.r dir.r mem.r \
	time.r process.r id.r intercept.r syscall.r syscommon.r \
	cfinish.r profdummy.r tidyup.r

all:	syslib.l cstart.r root.r
	cp cstart.r root.r ..

syslib.l: $(RFILES)
	cat $(RFILES) > syslib.l

cstart.r: cstart.a

root.r: root.a

clean:
	-$(RM) $(RFILES) syslib.l cstart.r root.r
