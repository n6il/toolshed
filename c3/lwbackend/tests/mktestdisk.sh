#!/bin/sh
# mktestdisk.sh - build the ctest init-data test module and an OS-9 disk image
# to run it under NitrOS-9 (USim, Pico-Thing L2).
#
# Usage:  sh mktestdisk.sh [TESTNAME] [OUT.dsk]   (default: ctest -> ./ctest_j0.dsk)
#         e.g.  sh mktestdisk.sh ftest             (-> ./ftest_j0.dsk)
#
# Requires: the c3 unix build (build/unix/{prep,comp,optx}); the runtime built
# by ../buildrt.sh (crt0.o + libc3lw.a); lwasm/lwlink; and Toolshed `os9`.
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
      "$CRT0" "$name.o" putstr.o "-L$lb" -lc3lw )

echo "building disk image $out ..."
rm -f "$out"
os9 format -l2097152 -q "$out" -n"C3TEST" >/dev/null
os9 copy "$work/$name" "$out,$name"
os9 attr -e -pe "$out,$name"                # REQUIRED: make it loadable
echo "done:"
os9 dir "$out" | tail -2
os9 attr "$out,$name"
