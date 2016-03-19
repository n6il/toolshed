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

all:
	$(foreach dir, $(DIRS), (cd $(dir); $(MAKE));)

install: all
	$(INSTALL) -d $(INSTALLDIR) $(DOCDIR)
	$(INSTALL) ar2/ar2 os9/os9 mamou/mamou cecb/cecb decb/decb \
	    tocgen/tocgen makewav/makewav $(INSTALLDIR)
	-$(INSTALL) cocofuse/cocofuse $(INSTALLDIR)
	$(INSTALL) -m 0644 doc/ToolShed.html $(DOCDIR)

package: INSTALLDIR=toolshed-$(VERSION)
package: DOCDIR = $(INSTALLDIR)
package: install
	tar czvf $(PACKAGENAME) toolshed-$(VERSION)
	rm -rf toolshed-$(VERSION)

clean:
	$(foreach dir, $(DIRS), (cd $(dir); $(MAKE) clean);)
	$(RM) $(PACKAGENAME)
