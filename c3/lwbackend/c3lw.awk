#!/usr/bin/awk -f
#
# c3lw.awk - translate c3 (Microware RMA / OS-9) assembly into lwasm syntax
# for the lwasm + lwlink OS-9-module backend.  Reads a .s on stdin, writes
# lwasm-syntax .s on stdout.
#
# What it does:
#   * psect NAME,...        -> section code      (code/text)
#     vsect / vsect dp      -> section bss        (static data; see caveat)
#     endsect               -> endsection
#     nam / ttl             -> dropped (comment)
#   * emits  EXPORT  for every non-temporary defined label, so symbols are
#     visible to other objects / crt0  (over-exports statics - see caveat);
#   * emits  EXTERN  for every referenced symbol not defined in this file
#     (library helpers like _stkcheck/ccmult, cross-module calls), which RMA
#     auto-externed via ROF but lwasm needs declared.
#
# Caveats (documented, not yet handled):
#   * vsect holding *initialised* data (fcb/fdb/fcc, e.g. "int x = 42;") is
#     emitted into a bss section, which lwasm rejects.  Uninitialised globals
#     (rmb) are fine.  Initialised globals need an init-image + copy in crt0.
#   * static functions/globals are over-exported (can clash if two modules
#     define the same static name).  The compiler-side backend would know.
#
# 6809/6309 register and directive names that are never symbol references:
BEGIN {
	split("a b d e f w q x y u s pc pcr cc dp v z zero", rr, " ")
	for (k in rr) reg[rr[k]] = 1
	split("section endsection export extern import equ rmb fcb fdb fcc fcn fqb fdbs nam ttl psect vsect endsect end org setdp align", dd, " ")
	for (k in dd) dirw[dd[k]] = 1
	n = 0
}
{ raw[n++] = $0 }

function tok_label(s,   t) {
	# a label is a token in column 0, terminated by ':' or whitespace
	if (s !~ /^[A-Za-z_.]/) return ""
	t = s
	sub(/[ \t:].*$/, "", t)
	return t
}

function collect_refs(s,   i, c, cur, inq) {
	# pull identifier tokens out of a line's operand area and mark referenced
	# (over-collecting registers/directives is harmless - they're filtered);
	# skip the contents of "..." strings so fcc text is not seen as symbols
	cur = ""; inq = 0
	for (i = 1; i <= length(s); i++) {
		c = substr(s, i, 1)
		if (inq) { if (c == "\"") inq = 0; continue }
		if (c == "\"") { if (cur != "") refmark(cur); cur = ""; inq = 1; continue }
		if (c ~ /[A-Za-z0-9_.]/) cur = cur c
		else { if (cur != "") refmark(cur); cur = "" }
	}
	if (cur != "") refmark(cur)
}
function refmark(t) {
	if (t ~ /^[0-9]/) return			# number / hex
	if (t ~ /^[0-9A-Fa-f]+$/) return
	if (reg[t] || dirw[t]) return
	referenced[t] = 1
}

END {
	# ---- pass 1: defined + referenced symbols ----
	for (i = 0; i < n; i++) {
		s = raw[i]
		lbl = tok_label(s)
		if (lbl != "") defined[lbl] = 1
		# strip the label, then the mnemonic, leaving only the operand
		rest = s
		if (s ~ /^[A-Za-z_.]/) sub(/^[^ \t:]+:?/, "", rest)	# drop label[:]
		sub(/^[ \t]+/, "", rest)				# trim
		mnem = rest; sub(/[ \t].*$/, "", mnem)			# mnemonic/directive
		if (mnem == "psect" || mnem == "vsect" || mnem == "endsect" \
		    || mnem == "nam" || mnem == "ttl") continue	# translated away
		oper = rest; sub(/^[^ \t]+/, "", oper)			# operand only
		collect_refs(oper)
	}

	# ---- emit EXTERN / EXPORT header ----
	# (use the "in" operator, not defined[t]: a bare subscript reference
	#  would auto-create the key in awk and pollute the export list)
	for (t in referenced)
		if (!(t in defined)) print "\textern\t" t
	for (t in defined)
		if (t !~ /^_[0-9]/ && t !~ /^_O[0-9]/ && t != "")
			print "\texport\t" t

	# ---- pass 2: translate body ----
	# RMA nests sections: psect opens the program (code) section; vsect opens
	# a data sub-section; endsect closes the innermost, returning to the
	# enclosing one (so the endsect after a vsect resumes code, and only the
	# final endsect - which closes the psect - ends the module).  lwasm's
	# section/endsection don't nest, so track a stack and re-emit "section
	# code" when returning to the psect.
	ssp = 0
	for (i = 0; i < n; i++) {
		s = raw[i]
		if (s ~ /^[ \t]+psect[ \t]/) {
			print "\tsection\tcode"; sstk[++ssp] = "code"; continue
		}
		if (s ~ /^[ \t]+vsect([ \t]|$)/) {
			print "\tsection\tbss"; sstk[++ssp] = "bss"; continue
		}
		if (s ~ /^[ \t]+endsect([ \t]|$)/) {
			ssp--
			if (ssp > 0) print "\tsection\t" sstk[ssp]	# back to enclosing
			else print "\tendsection"			# closed the psect
			continue
		}
		if (s ~ /^[ \t]+(nam|ttl)([ \t]|$)/) { sub(/^[ \t]+/, "", s); print "* " s; continue }
		# Warn on *initialised* global data: it lands in the bss-named data
		# section, whose initial contents lwlink's OS-9 target discards (it
		# uses only the size).  Until crt0 copies an init-image, initialised
		# globals link but are not initialised at runtime.
		if (ssp > 0 && sstk[ssp] == "bss" && s ~ /^[^ \t]*[ \t]+(fcb|fdb|fcc|fcn|fqb)([ \t]|$)/)
			print "c3lw: warning: initialised global data is not yet initialised at runtime: " s > "/dev/stderr"
		print s
	}
}
