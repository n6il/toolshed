#!/bin/sh
# buildrt.sh - build the c3 lwasm/lwlink backend runtime: crt0.o plus a
# library archive (c3lw.l) of the C helper routines the compiler calls
# (ccmult, ccdiv, shifts, the long/float math, ...).  Each helper is an
# original Microware-RMA .a source run through the c3lw.awk translation
# filter, assembled with lwasm, and archived with lwar.
#
# Usage:  sh buildrt.sh            (run from c3/lwbackend)
# Output: lwbackend/crt0.o, lwbackend/c3lw.l
set -e
here=$(cd "$(dirname "$0")" && pwd)
c3=$(cd "$here/.." && pwd)
filter="$here/c3lw.awk"
out="$here/obj"
mkdir -p "$out"

asm() {   # asm <name> <source.a>
	awk -f "$filter" < "$2" > "$out/$1.s"
	if lwasm --format=obj -o "$out/$1.o" "$out/$1.s" 2>"$out/$1.err"; then
		echo "  ok   $1" >&2
		echo "$out/$1.o"
	else
		echo "  SKIP $1 ($(head -1 "$out/$1.err"))" >&2
	fi
}

echo "crt0:"
lwasm --format=obj -o "$here/crt0.o" "$here/crt0.s"
echo "  ok   crt0"

# Helper routines to attempt: the integer + long math.  Each is attempted and
# included if it assembles.  The float/double helpers are NOT here: they are
# fpwrap.a (MC6839, IEEE-754), which buildlibc.sh puts in libc3.a.  The old
# Microware-format fmath/cfloats.a doesn't match the compiler's IEEE constants.
objs=""
echo "runtime helpers:"
for src in \
	"$c3"/lib/imath/ccmult.a \
	"$c3"/lib/imath/ccdiv.a \
	"$c3"/lib/imath/ccmod.a \
	"$c3"/lib/imath/cshifts.a \
	"$c3"/lib/imath/abs.a \
	"$c3"/lib/imath/minmax.a \
	"$c3"/lib/imath/uminmax.a \
	"$c3"/lib/imath/clcommon.a \
	"$c3"/lib/imath/claddsub.a \
	"$c3"/lib/imath/clmul.a \
	"$c3"/lib/imath/cldiv.a \
	"$c3"/lib/imath/clshifts.a \
	"$c3"/lib/imath/clcompare.a \
	"$c3"/lib/imath/clmove.a \
	"$c3"/lib/imath/clnegcompl.a \
	"$c3"/lib/imath/clbits.a \
	"$c3"/lib/imath/clincdec.a \
	"$c3"/lib/imath/clconvert.a
do
	[ -f "$src" ] || continue
	name=$(basename "$src" .a)
	o=$(asm "$name" "$src" || true)
	[ -n "$o" ] && objs="$objs $o"
done

echo "archiving c3lw.l:"
rm -f "$here/libc3lw.a"
# shellcheck disable=SC2086
lwar -c "$here/libc3lw.a" $objs
echo "  libc3lw.a: $(echo $objs | wc -w | tr -d ' ') modules"
echo "done."
