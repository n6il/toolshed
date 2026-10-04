#!/bin/sh
# mktestdisk.sh - build the ctest init-data test module and an OS-9 disk image
# to run it under NitrOS-9 (USim, Pico-Thing L2).
#
# Usage:  sh mktestdisk.sh [TESTNAME] [OUT.dsk]   (default: ctest -> ./ctest_j0.dsk)
#         e.g.  sh mktestdisk.sh ftest             (-> ./ftest_j0.dsk)
#
# Requires: the c3 unix build (build/unix/{prep,comp,optx}); the runtime built
# by ../buildrt.sh (crt0.o + libc3lw.a) and ../buildlibc.sh (libc3.a and the
# fpdummy.o/fpinit.o FP startup objects);
# lwasm/lwlink; and Toolshed `os9`.
#
# ftest uses floating point, so it links fpinit.o (the MC6839 startup) and needs
# the FPO9 module at run time.  Set NFP09 to an nfp09 checkout
# (https://github.com/markrvmurray/nfp09.git) to put ROM/Float09.bin on the
# disk as FPO9; at run time it must be resident or in /DD/Modules.
#
# Produces a hard-drive image holding the `ctest` module with the EXECUTE
# attribute set (os9 copy does NOT set it, and OS-9 refuses to load a module
# file without it -> Error #214/#215).  See README.md for running it: copy the
# module onto a NitrOS-9 boot disk, or mount the image as /j0 on a boot that
# has the J0 descriptor.
set -e
here=$(cd "$(dirname "$0")" && pwd)
lb=$(cd "$here/.." && pwd)                 # lwbackend/
c3=$(cd "$lb/.." && pwd)                    # c3/
B="$c3/build/unix"
PREP="$B/prep/c3prep"; COMP="$B/comp/c3comp"; OPTX="$B/optx/c3optx"
FILTER="$lb/c3lw.awk"; SCRIPT="$lb/os9c3.ld"; CRT0="$lb/crt0.o"

name=${1:-ctest}                            # which tests/<name>.c to build
fp="$lb/fpdummy.o"                          # FP programs opt in to fpinit.o
case "$name" in ftest) fp="$lb/fpinit.o" ;; esac
out=${2:-"$here/${name}_j0.dsk"}
[ -f "$here/$name.c" ] || { echo "no such test: $here/$name.c" >&2; exit 2; }
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

echo "compiling $name.c ..."
"$PREP" -DOS9 "$here/$name.c" | "$COMP" | "$OPTX" | awk -f "$FILTER" > "$work/$name.lw.s"
lwasm --format=obj -o "$work/$name.o"  "$work/$name.lw.s"
lwasm --format=obj -o "$work/putstr.o" "$here/putstr.s"

echo "linking $name module ..."
# Link from the work dir with a bare output name so the embedded module name is
# clean (e.g. "ctest"), not a host path.
( cd "$work" && lwlink --format=os9 -s "$SCRIPT" -o "$name" \
      "$CRT0" "$name.o" putstr.o $fp "-L$lb" -lc3 -lc3lw )

echo "building disk image $out ..."
rm -f "$out"
os9 format -l2097152 -q "$out" -n"C3TEST" >/dev/null
os9 copy "$work/$name" "$out,$name"
os9 attr -e -pe "$out,$name"                # REQUIRED: make it loadable
if [ "$fp" = "$lb/fpinit.o" ]; then
	if [ -n "$NFP09" ] && [ -f "$NFP09/ROM/Float09.bin" ]; then
		os9 copy "$NFP09/ROM/Float09.bin" "$out,FPO9"
		os9 attr -e -pe "$out,FPO9"
	else
		echo "warning: NFP09 not set - no FPO9 on the disk; $name needs it" >&2
	fi
fi
echo "done:"
os9 dir "$out" | tail -2
os9 attr "$out,$name"
