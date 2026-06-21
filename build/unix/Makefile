# Makefile for ToolShed
include rules.mak

vpath %.c ../../mamou:../../os9

PACKAGENAME	= toolshed-$(VERSION).tgz
# Install location.  PREFIX is the install prefix (/usr/local by default);
# DESTDIR is an optional staging root prepended to every installed path for
# package building.  The two are orthogonal - a distro package is built with
# e.g. `make install PREFIX=/usr DESTDIR=<pkgdir>`.
PREFIX		?= /usr/local
INSTALLDIR	= $(DESTDIR)$(PREFIX)/bin
DOCDIR		= $(DESTDIR)$(PREFIX)/share/toolshed
C3BASE		= $(PREFIX)/share/toolshed
DIRS		= libtoolshed libnative libcecb librbf libcoco libdecb libmisc libsys \
		ar2 os9 mamou decb tocgen cocofuse makewav libcecb cecb doc

# The c3 (C-Cubed) compiler has its own build tree; build it as part of "all".
C3DIR		= ../../c3/build/unix
C3LWDIR		= ../../c3/lwbackend
C3DEFSDIR	= ../../c3/defs
# The c3 driver finds its preprocessor headers under $(C3BASE)/defs; C3BASE is
# the toolshed share dir (no more /opt/c3).  The driver is built with this as
# its compiled-in default (C3BASE_DEFAULT) so it works with no environment;
# C3BASEDIR is where the install stages those headers (DESTDIR-aware).
C3BASEDIR	= $(DESTDIR)$(C3BASE)

all:
	$(foreach dir, $(DIRS), (cd $(dir); $(MAKE));)
	$(MAKE) -C $(C3DIR) all C3BASE_DEFAULT=$(C3BASE)

install: all
	$(INSTALL) -d $(INSTALLDIR) $(DOCDIR)
	$(INSTALL) ar2/ar2 os9/os9 mamou/mamou cecb/cecb decb/decb \
	    tocgen/tocgen makewav/makewav $(INSTALLDIR)
	-$(INSTALL) cocofuse/cocofuse $(INSTALLDIR)
	# c3comp is the single-pass compiler (the driver's default); c3comp1 and
	# c3comp2 are the two-pass front/back ends the driver uses for "c3 -2".
	$(INSTALL) $(C3DIR)/exec/c3 $(C3DIR)/prep/c3prep $(C3DIR)/comp/c3comp \
	    $(C3DIR)/comp1/c3comp1 $(C3DIR)/comp2/c3comp2 \
	    $(C3DIR)/optx/c3optx $(C3DIR)/lorder/lorder $(C3DIR)/lsplit/lsplit \
	    $(INSTALLDIR)
	# lwasm/lwlink back end: the shims the c3 driver calls, the standalone
	# c3lw driver, and (if built by buildrt.sh) the runtime they locate
	# beside themselves.  The "-" lets install proceed when the runtime
	# artifacts have not been built (no lwtools).
	$(INSTALL) $(C3LWDIR)/c3-as $(C3LWDIR)/c3-ld $(C3LWDIR)/c3lw $(INSTALLDIR)
	$(INSTALL) -m 0644 $(C3LWDIR)/c3lw.awk $(C3LWDIR)/os9c3.ld $(INSTALLDIR)
	-$(INSTALL) -m 0644 $(C3LWDIR)/crt0.o $(C3LWDIR)/libc3.a $(C3LWDIR)/libc3lw.a $(INSTALLDIR)
	# c3 preprocessor headers, under $(C3BASE)/defs so #include resolves
	$(INSTALL) -d $(C3BASEDIR)/defs $(C3BASEDIR)/defs/cgfx
	$(INSTALL) -m 0644 $(C3DEFSDIR)/*.h $(C3BASEDIR)/defs
	$(INSTALL) -m 0644 $(C3DEFSDIR)/cgfx/*.h $(C3BASEDIR)/defs/cgfx
	$(INSTALL) -m 0644 doc/ToolShed.html $(DOCDIR)

package: INSTALLDIR=toolshed-$(VERSION)
package: DOCDIR = $(INSTALLDIR)
# Stage the c3 headers inside the package tree too; without this override
# C3BASEDIR resolves to an absolute path, so the defs would miss the tarball
# (and write outside it).
package: C3BASEDIR = toolshed-$(VERSION)/share/toolshed
package: install
	tar czvf $(PACKAGENAME) toolshed-$(VERSION)
	rm -rf toolshed-$(VERSION)

clean:
	$(foreach dir, $(DIRS), (cd $(dir); $(MAKE) clean);)
	$(MAKE) -C $(C3DIR) clean
	$(RM) $(PACKAGENAME)
