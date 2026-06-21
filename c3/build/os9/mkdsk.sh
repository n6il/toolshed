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
C3MODS="$here/exec/c3 $here/prep/c3prep $here/comp1/c3comp1 $here/comp2/c3comp2 \
        $here/optx/c3optx $here/lorder/lorder $here/lsplit/lsplit $here/make/make"
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

# library sources, pre-substituted for rma the same way the host build does:
# strip `use'/IFP1, then os9defs.sed (syscall/error-code names -> codes) and
# libdefs.sed (stdio FILE offsets -> literals), so on-target rma needs none of
# the lwasm-flavoured defs files (which it cannot assemble).
STRIP='/^[ \t]*[Ii][Ff][Pp]1/,/^[ \t]*[Ee][Nn][Dd][Cc]/d;/^[ \t]*[Uu][Ss][Ee][ \t]/d'
ppa() { sed -e "$STRIP" "$1" | sed -f "$c3/lwbackend/os9defs.sed" | sed -f "$c3/lwbackend/libdefs.sed"; }
for d in lib.a sys.a; do
	mkdir -p "$stage/src/lib/$d"
	for f in "$c3/lib/$d"/*.a; do ppa "$f" > "$stage/src/lib/$d/$(basename "$f")"; done
done
cp "$c3/lib/lib.a/chcodes.c" "$stage/src/lib/lib.a/"	# C: compiled (c3 -2), not rma'd

# long (32-bit) integer arithmetic (fseek/scanf reference _ladd/_lcmpr/...)
mkdir -p "$stage/src/imath"
for f in claddsub clbits clcommon clcompare clconvert cldiv \
         clincdec clmove clmul clnegcompl clshifts \
         ccmult ccmod ccdiv cshifts rpterr; do
	ppa "$c3/lib/imath/$f.a" > "$stage/src/imath/$f.a"
done

# back-end standalones not in the stock lib (getpid/attach/getenv/mkstemp and
# the no-op FP stub _fpinit/_fpterm), plus a minimal valid ROF for sys.l
mkdir -p "$stage/src/back"
for f in getpid attach getenv mkstemp fpdummy; do
	ppa "$c3/lwbackend/$f.a" > "$stage/src/back/$f.a"
done
printf ' psect sysstub_a,0,0,0,0,0\n endsect\n' > "$stage/src/sysstub.a"

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
echo "  CMDS:    c3 c3prep c3comp1 c3comp2 c3optx lorder lsplit make + $RUTILS"
echo "  defs:    c3 v2-ABI headers"
echo "  src:     compiler sources + rma-ready library sources (lib/ imath/ back/), mklibs"
echo "  lib:     (build on-target:  chx /dd/cmds; chd /dd/src; shell </dd/src/mklibs)"
echo "  Modules: FPO9 (MC6839 floating point)"
