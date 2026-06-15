#!/bin/sh
# mkdsk.sh - build a complete c3-on-NitrOS-9 distribution disk image.
#
# Lays out, on one OS-9 image (mounted as the driver's default base /dd):
#   CMDS/   the cross-built c3 tools  +  the Microware r-utils (rma rlink mold
#           rdump) that the on-target driver shells out to (ASMBNAME=rma,
#           LINKNAME=rlink); every module gets the OS-9 execute attribute.
#   defs/   c3's own v2-ABI headers (NOT CoCoC's - different ABI).
#   lib/    empty; populated on first boot by `src/mklibs` (build-on-target).
#   src/    the c3 compiler + library sources, and the mklibs procedure.
#
# Text trees (defs, src) are copied with `os9 dsave -l` so the host's LF line
# endings become OS-9 CR; command modules are copied binary.
#
# Run from build/os9 (the makefile does `cd $(dir)` for the per-tool builds,
# but invokes this from build/os9 itself).  Override via the environment:
#   DSK     output image           (default c3tools.dsk, in build/os9)
#   DSKNAME OS-9 volume name        (default C3TOOLS)
#   LSECTORS flat OS-9 logical sectors (256B each; default 16384 = 4MB)
#   COCOC   CoCo C checkout (https://github.com/DrPitre/CoCoC.git); required,
#           supplies the r-utils rma rlink mold rdump in $COCOC/Cmds
#   NFP09   nfp09 checkout (https://github.com/markrvmurray/nfp09.git); supplies
#           the MC6839 FP module as $NFP09/ROM/Float09.bin
#   FPO9    the MC6839 FP module itself (default $NFP09/ROM/Float09.bin)
set -e

here=$(cd "$(dirname "$0")" && pwd)		# build/os9
c3=$(cd "$here/../.." && pwd)			# repo c3 root

DSK=${DSK:-$here/c3tools.dsk}
DSKNAME=${DSKNAME:-C3TOOLS}
LSECTORS=${LSECTORS:-16384}			# flat OS-9 logical sectors (256B); 4MB
FPO9=${FPO9:-${NFP09:+$NFP09/ROM/Float09.bin}}

# c3 command modules (built by the per-tool makefiles) -> CMDS
C3MODS="$here/exec/c3 $here/prep/c3prep $here/comp/c3comp \
        $here/optx/c3optx $here/lorder/lorder $here/lsplit/lsplit"
# Microware assembler / linker / librarian / ROF dumper -> CMDS
RUTILS="rma rlink mold rdump"

# ---- preflight --------------------------------------------------------------
for m in $C3MODS; do
	[ -f "$m" ] || { echo "mkdsk: missing $m - run 'make' first" >&2; exit 1; }
done
[ -n "$COCOC" ] || {
	echo "mkdsk: set COCOC to a CoCo C checkout" \
	     "(https://github.com/DrPitre/CoCoC.git; it ships rma/rlink/mold/rdump)" >&2
	exit 1
}
missing=
for u in $RUTILS; do
	[ -f "$COCOC/Cmds/$u" ] || missing="$missing $u"
done
[ -z "$missing" ] || {
	echo "mkdsk: r-utils not found in $COCOC/Cmds:$missing" >&2
	echo "mkdsk: set COCOC to a CoCo C checkout" \
	     "(https://github.com/DrPitre/CoCoC.git; it ships rma/rlink/mold/rdump)" >&2
	exit 1
}

# ---- format -----------------------------------------------------------------
echo "mkdsk: formatting $DSK ($LSECTORS logical sectors)"
rm -f "$DSK"
os9 format "$DSK" -e -l"$LSECTORS" -q -n"$DSKNAME"

# ---- CMDS: command modules (binary) + execute attribute ---------------------
echo "mkdsk: CMDS"
os9 makdir "$DSK,CMDS"
for m in $C3MODS; do
	b=$(basename "$m")
	os9 copy -r "$m" "$DSK,CMDS/$b"
	os9 attr -e -pe "$DSK,CMDS/$b"
done
for u in $RUTILS; do
	os9 copy -r "$COCOC/Cmds/$u" "$DSK,CMDS/$u"
	os9 attr -e -pe "$DSK,CMDS/$u"
done

# ---- stage the text trees (defs, src) into a temp, then dsave with -l --------
stage=$here/.diststage
rm -rf "$stage"

# headers: c3's own v2-ABI defs + the ROF header
mkdir -p "$stage/defs"
cp "$c3"/defs/*.h "$stage/defs/"
cp "$c3"/include/rof.h "$stage/defs/"

# compiler sources
for d in prep comp optexp exec; do
	mkdir -p "$stage/src/$d"
	cp "$c3/$d"/*.c "$stage/src/$d/" 2>/dev/null || true
	cp "$c3/$d"/*.h "$stage/src/$d/" 2>/dev/null || true
done

# library sources (the v2-ABI C library we build on-target)
for d in lib.a sys.a lib.c; do
	mkdir -p "$stage/src/lib/$d"
	cp "$c3/lib/$d"/* "$stage/src/lib/$d/" 2>/dev/null || true
done

# MC6839 floating-point sources (mklibs assembles these into clib.l on-target)
mkdir -p "$stage/src/fp"
cp "$c3/lwbackend/fpwrap.a" "$c3/lwbackend/fpinit.a" "$stage/src/fp/"

# the on-target library-build procedure
cp "$here/mklibs" "$stage/src/mklibs"

echo "mkdsk: defs/ + src/ (text, LF->CR)"
os9 dsave -l -e "$stage" "$DSK,"

# ---- lib: empty, filled in on-target by src/mklibs --------------------------
os9 makdir "$DSK,lib"

# ---- Modules: the MC6839 FP package, where the driver's F$Link/F$Load find it
os9 makdir "$DSK,Modules"
if [ -f "$FPO9" ]; then
	os9 copy -r "$FPO9" "$DSK,Modules/FPO9"
	echo "mkdsk: installed FPO9 (MC6839) -> Modules/"
else
	echo "mkdsk: WARNING: MC6839 module ${FPO9:-(NFP09 unset)} not found - set NFP09" \
	     "to an nfp09 checkout, or install FPO9 into Modules/ for FP" >&2
fi

rm -rf "$stage"
echo "mkdsk: done -> $DSK"
echo "  CMDS:    c3 c3prep c3comp c3optx lorder lsplit + $RUTILS"
echo "  defs:    c3 v2-ABI headers"
echo "  src:     compiler + library sources (incl fp/), mklibs"
echo "  lib:     (build on-target:  chd /dd/src; mklibs)"
echo "  Modules: FPO9 (MC6839 floating point)"
