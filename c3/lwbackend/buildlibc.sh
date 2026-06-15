#!/bin/sh
# buildlibc.sh - port the c3 C library to an OS-9 archive (libc3.a) with the
# cross-toolchain, so cross-compiled programs (the c3 tools themselves) can link
# stdio / string / process / memory routines.
#
# .a (RMA) sources: rewrite OS-9 syscall names to codes (os9defs.sed), run the
# c3lw.awk filter, assemble with lwasm.  .c sources: full c3 pipeline.  Files
# that assemble are archived; failures are reported (the to-do list).
#
# Usage:  sh buildlibc.sh        (run from c3/lwbackend)
# Output: lwbackend/libc3.a
set -e
here=$(cd "$(dirname "$0")" && pwd)
c3=$(cd "$here/.." && pwd)
B="$c3/build/unix"
PREP="$B/prep/c3prep"; COMP="$B/comp/c3comp"; OPTX="$B/optx/c3optx"
FILTER="$here/c3lw.awk"; SED="$here/os9defs.sed"; IODEFS="$here/libdefs.sed"
out="$here/libcobj"; mkdir -p "$out"
objs=""

# strip RMA-isms lwasm can't take: the `ifp1 ... endc` pass-1 header block (and
# any bare `use` include) - the headers it pulls in are supplied separately
# (os9defs.sed substitutions, the stdio_defs object).
STRIP='/^[ \t]*[Ii][Ff][Pp]1/,/^[ \t]*[Ee][Nn][Dd][Cc]/d;/^[ \t]*[Uu][Ss][Ee][ \t]/d'

asma() {	# asma <name> <src.a>   ($3 = "io" also applies libdefs.sed for stdio)
	if [ "$3" = io ]; then
		sed -e "$STRIP" "$2" | sed -f "$SED" | sed -f "$IODEFS" | awk -f "$FILTER" > "$out/$1.s" 2>/dev/null
	else
		sed -e "$STRIP" "$2" | sed -f "$SED" | awk -f "$FILTER" > "$out/$1.s" 2>/dev/null
	fi
	if lwasm --format=obj -o "$out/$1.o" "$out/$1.s" 2>"$out/$1.err"; then
		echo "  ok   $1" >&2; objs="$objs $out/$1.o"
	else
		echo "  SKIP $1 ($(head -1 "$out/$1.err"))" >&2
	fi
}
asmc() {	# asmc <name> <src.c>
	# Run c3prep from c3/ with paths relative to it, so the include paths stay
	# well inside c3prep's FNAMSIZE limit however deep the checkout is.
	( cd "$c3" && "$PREP" -DOS9 -v=defs -v="$(dirname "${2#"$c3"/}")" \
	    "${2#"$c3"/}" ) 2>/dev/null \
	    | "$COMP" 2>/dev/null | "$OPTX" 2>/dev/null | awk -f "$FILTER" \
	    > "$out/$1.s" 2>/dev/null
	if lwasm --format=obj -o "$out/$1.o" "$out/$1.s" 2>"$out/$1.err"; then
		echo "  ok   $1" >&2; objs="$objs $out/$1.o"
	else
		echo "  SKIP $1 ($(head -1 "$out/$1.err"))" >&2
	fi
}

echo "string:" >&2
for s in strlen strncmp strncpy strncat strings strass strclr strcmp index reverse strhcpy; do
	[ -f "$c3/lib/lib.a/$s.a" ] && asma "$s" "$c3/lib/lib.a/$s.a"
done

echo "syscalls (sys.a):" >&2
for s in syscommon write read open create misc mem access process intercept \
         lseek dir stat time signal; do
	[ -f "$c3/lib/sys.a/$s.a" ] && asma "$s" "$c3/lib/sys.a/$s.a"
done
asma getpid "$here/getpid.a"		# standalone (vs all of id.a)
asma attach "$here/attach.a"		# I$Attach/I$Detach wrappers (not in stock lib)
asma getenv "$here/getenv.a"		# environment stub (no OS-9 environ)
asma mkstemp "$here/mkstemp.a"		# mkstemp -> mktemp (not in stock lib)

echo "stdio (lib.a):" >&2
# stdio routines: the FILE struct offsets/flags are substituted as literals
# (libdefs.sed) since lwlink can't share absolute-equate exports across objects.
for s in iob_data fopen getc putc puts fseek setbuf fread fwrite mktemp gets setbase printf scanf; do
	[ -f "$c3/lib/lib.a/$s.a" ] && asma "$s" "$c3/lib/lib.a/$s.a" io
done
# printf's %ld formatter (real) + %f formatter (stub: skips the arg, no FP dep)
asma pflong "$c3/lib/lib.a/pflinit.a"
asma pffloat "$c3/lib/lib.a/pffdummy.a"

echo "process / memory:" >&2
asma cfinish  "$c3/lib/sys.a/cfinish.a"
asma profdummy "$c3/lib/sys.a/profdummy.a"
# crt0 calls _fpinit/_fpterm; the cross-built tools use no FP, so the no-op stub
# is the default.  FP-using programs link fpinit.a instead (F$Links MC6839 FPO9).
asma fpdummy  "$here/fpdummy.a"
asma memory   "$c3/lib/lib.a/memory.a" io
asma calloc   "$c3/lib/lib.a/calloc.a"
asma atoi     "$c3/lib/lib.a/atoi.a"
asma xtoa     "$c3/lib/lib.a/xtoa.a"		# itoa/utoa (pulled in by mktemp)
[ -f "$c3/lib/lib.a/chcodes.c" ] && asmc chcodes "$c3/lib/lib.a/chcodes.c"

echo "archiving libc3.a:" >&2
rm -f "$here/libc3.a"
# shellcheck disable=SC2086
lwar -c "$here/libc3.a" $objs
echo "  libc3.a: $(echo $objs | wc -w | tr -d ' ') modules" >&2
