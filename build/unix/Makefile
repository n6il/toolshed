# Makefile for ToolShed
include rules.mak

vpath %.c ../../mamou:../../os9

PACKAGENAME	= toolshed-$(VERSION).tgz
ifneq ($(DESTDIR),)
INSTALLDIR	= $(DESTDIR)/usr/bin
DOCDIR		= $(DESTDIR)/usr/share/doc/toolshed
else
INSTALLDIR	= /usr/local/bin
DOCDIR		= /usr/local/share/toolshed
endif
DIRS		= libtoolshed libnative libcecb librbf libcoco libdecb libmisc libsys \
		ar2 os9 mamou decb tocgen cocofuse makewav libcecb cecb doc

# The c3 (C-Cubed) compiler has its own build tree; build it as part of "all".
C3DIR		= ../../c3/build/unix
C3LWDIR		= ../../c3/lwbackend
C3DEFSDIR	= ../../c3/defs
# The c3 driver finds preprocessor headers under $(C3BASE)/defs; C3BASE
# defaults to /opt/c3 (see sysdrive() in exec/c3.c), a deliberately short
# path - long header paths overflow fixed buffers in the K&R sources.
C3BASEDIR	= $(DESTDIR)/opt/c3

all:
	$(foreach dir, $(DIRS), (cd $(dir); $(MAKE));)
	$(MAKE) -C $(C3DIR) all

install: all
	$(INSTALL) -d $(INSTALLDIR) $(DOCDIR)
	$(INSTALL) ar2/ar2 os9/os9 mamou/mamou cecb/cecb decb/decb \
	    tocgen/tocgen makewav/makewav $(INSTALLDIR)
	-$(INSTALL) cocofuse/cocofuse $(INSTALLDIR)
	$(INSTALL) $(C3DIR)/exec/c3 $(C3DIR)/prep/c3prep $(C3DIR)/comp/c3comp \
	    $(C3DIR)/optx/c3optx $(C3DIR)/lorder/lorder $(C3DIR)/lsplit/lsplit \
	    $(INSTALLDIR)
	# lwasm/lwlink back end: the shims the c3 driver calls, the standalone
	# c3lw driver, and (if built by buildrt.sh) the runtime they locate
	# beside themselves.  The "-" lets install proceed when the runtime
	# artifacts have not been built (no lwtools).
	$(INSTALL) $(C3LWDIR)/c3-as $(C3LWDIR)/c3-ld $(C3LWDIR)/c3lw $(INSTALLDIR)
	$(INSTALL) -m 0644 $(C3LWDIR)/c3lw.awk $(C3LWDIR)/os9c3.ld $(INSTALLDIR)
	-$(INSTALL) -m 0644 $(C3LWDIR)/crt0.o $(C3LWDIR)/libc3lw.a $(INSTALLDIR)
	# c3 preprocessor headers, under $(C3BASE)/defs so #include resolves
	$(INSTALL) -d $(C3BASEDIR)/defs $(C3BASEDIR)/defs/cgfx
	$(INSTALL) -m 0644 $(C3DEFSDIR)/*.h $(C3BASEDIR)/defs
	$(INSTALL) -m 0644 $(C3DEFSDIR)/cgfx/*.h $(C3BASEDIR)/defs/cgfx
	$(INSTALL) -m 0644 doc/ToolShed.html $(DOCDIR)

package: INSTALLDIR=toolshed-$(VERSION)
package: DOCDIR = $(INSTALLDIR)
package: install
	tar czvf $(PACKAGENAME) toolshed-$(VERSION)
	rm -rf toolshed-$(VERSION)

clean:
	$(foreach dir, $(DIRS), (cd $(dir); $(MAKE) clean);)
	$(MAKE) -C $(C3DIR) clean
	$(RM) $(PACKAGENAME)
