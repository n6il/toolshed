#!/usr/bin/awk -f
#
# c3lw.awk - translate c3 (Microware RMA / OS-9) assembly into lwasm syntax
# for the lwasm + lwlink OS-9-module backend.  Reads a .s on stdin, writes
# lwasm-syntax .s on stdout.
#
# Code/text:
#   * psect NAME,...        -> section code
#     endsect (closing it)  -> endsection
#   * nam / ttl             -> dropped (comment)
#   * EXPORT every non-temporary defined label; EXTERN every referenced symbol
#     not defined here (RMA auto-externed these via ROF; lwasm needs them
#     declared).
#
# Data (vsect):  c3 keeps statics/globals in vsects, addressed "name,y" with Y
# the data base.  An OS-9 module's data area is allocated (not loaded) by
# F$Fork, so initialised values must be carried in the loaded module body and
# copied in at startup by crt0.  This filter splits each vsect into:
#
#   * a reservation in "bss"   (the runtime data area; sizes only)
#   * for *initialised* vsects, an image in "data" (loaded) holding the initial
#     bytes, plus a copy record in "irec" (src/end/dest) that crt0 replays, plus
#     relocation entries for pointer initialisers:
#       - "ireloc"   pointer into the data area  (crt0 adds the data base Y)
#       - "icreloc"  pointer into the module body, e.g. a string literal or
#                    function address              (crt0 adds the module base)
#   Each data symbol becomes an equate giving its data-area offset, derived from
#   its position in the image (so lwasm computes the layout, not this script).
#   A vsect with no initialisers (only rmb/rzb) skips the image entirely and is
#   reserved straight into bss - crt0 zeroes the whole data area, so rmb/rzb
#   both read back as 0 with no module-space cost.
#
# See os9c3.ld (the matching lwlink script) and crt0.s.

BEGIN {
	split("a b d e f w q x y u s pc pcr cc dp v z zero", rr, " ")
	for (k in rr) reg[rr[k]] = 1
	split("section endsection export extern import equ rmb rzb fcb fdb fcc fcn fqb fdbs csect nam ttl psect vsect endsect end org setdp align", dd, " ")
	for (k in dd) dirw[dd[k]] = 1
	n = 0
	pfx = "anon"
	gotpfx = 0
}
{ raw[n++] = $0 }

function tok_label(s,   t) {
	# a label is a token in column 0, terminated by ':' or whitespace
	if (s !~ /^[A-Za-z_.]/) return ""
	t = s
	sub(/[ \t:].*$/, "", t)
	return t
}
function trim(s) { sub(/^[ \t]+/, "", s); sub(/[ \t]+$/, "", s); return s }

function collect_refs(s,   i, c, cur, inq, sep, started) {
	# pull identifier tokens out of a line's operand area and mark referenced.
	# `sep` is the punctuation char immediately before the current token, used
	# to tell a hex/binary literal ($CC, %01) from an identifier (cc, add) -
	# both look like hex once the leading $ / % is split off.  The operand is a
	# single whitespace-free run (quoted fcc strings excepted); once it ends, an
	# unquoted space begins an RMA inline comment, so stop - otherwise comment
	# words would be collected as bogus external references.
	cur = ""; inq = 0; sep = ""; started = 0
	for (i = 1; i <= length(s); i++) {
		c = substr(s, i, 1)
		if (inq) { if (c == "\"") inq = 0; continue }
		if (c == " " || c == "\t") {
			if (cur != "") { refmark(cur, sep); cur = "" }
			if (started) break		# operand finished -> rest is comment
			sep = c
			continue
		}
		started = 1
		if (c == "\"") { if (cur != "") refmark(cur, sep); cur = ""; sep = c; inq = 1; continue }
		if (c ~ /[A-Za-z0-9_.]/) cur = cur c
		else { if (cur != "") refmark(cur, sep); cur = ""; sep = c }
	}
	if (cur != "") refmark(cur, sep)
}
function refmark(t, sep) {
	if (sep == "$" || sep == "%") return		# $hex / %binary literal
	if (t ~ /^[0-9]/) return			# decimal literal
	# A `<`/`>` prefix forces a direct/extended memory operand, so the token is
	# a symbol even when it spells a register name (e.g. the global `<cc` vs the
	# CC register) - take it before the register/directive filter.
	if (sep == "<" || sep == ">") { referenced[t] = 1; return }
	# Only filter register names here, NOT directive names: a directive only
	# ever appears as a mnemonic, so a directive-spelled token in an operand
	# (e.g. `lbsr vsect` calling the compiler's own vsect() function) is a
	# symbol that must be externed.
	if (reg[t]) return				# 6809 register
	referenced[t] = 1
}

# true if an fdb/fqb operand is a relocatable symbol rather than a literal
function is_symbol(op) {
	if (op == "") return 0
	if (op ~ /^[-+]?\$[0-9A-Fa-f]/) return 0	# $hex
	if (op ~ /^[-+]?%[01]/) return 0		# %binary
	if (op ~ /^[-+]?[0-9]/) return 0		# decimal / expression of numbers
	if (op ~ /[A-Za-z_.]/) return 1			# contains an identifier
	return 0
}

# split "tab,42,msg" into ops[1..]; returns count.  fdb/fqb operands are simple
# values separated by commas (no quoted strings), so a plain comma split is safe.
function split_ops(s, ops,   i, c, cur, m) {
	m = 0; cur = ""
	for (i = 1; i <= length(s); i++) {
		c = substr(s, i, 1)
		if (c == ",") { ops[++m] = trim(cur); cur = "" }
		else cur = cur c
	}
	if (cur != "" || m > 0) ops[++m] = trim(cur)
	return m
}

# break a buffered source line into label / mnemonic / operand
function parse_line(s) {
	pl_lbl = tok_label(s)
	if (pl_lbl != "") sub(/^[^ \t:]+:?/, "", s)
	s = trim(s)
	pl_mnem = s; sub(/[ \t].*$/, "", pl_mnem)
	pl_oper = s; sub(/^[^ \t]*[ \t]*/, "", pl_oper); pl_oper = trim(pl_oper)
}

# value of an rmb-style size operand (decimal or $hex)
function csize(op,   v, i, c, h) {
	if (op ~ /^\$[0-9A-Fa-f]+$/) {
		v = 0
		for (i = 2; i <= length(op); i++) {
			c = tolower(substr(op, i, 1))
			h = index("0123456789abcdef", c) - 1
			v = v * 16 + h
		}
		return v
	}
	return op + 0
}

# ---- flush a buffered csect (frame/DP work area) -----------------------------
# Each `name rmb N` becomes `name equ <running offset>`; the storage itself is
# reserved on the stack at run time, so these are pure offsets.  vb[1..vbn].
function flush_csect(   i, off, oper, sign, rest, v) {
	off = 0
	split("", csym)				# csect label -> offset (cleared per csect)
	for (i = 1; i <= vbn; i++) {
		parse_line(vb[i])
		if (pl_mnem == "nam" || pl_mnem == "ttl") continue
		if (pl_mnem == "rmb") {
			if (pl_lbl != "") { print pl_lbl "\tequ\t" off; csym[pl_lbl] = off }
			off += csize(pl_oper)
		} else if (pl_lbl != "" && (pl_mnem == "" || \
		           ((pl_mnem == "equ" || pl_mnem == "set") && pl_oper ~ /^\*/))) {
			# location-counter offsets within the frame: `*`, `*-label`,
			# `*+label`, `*-N`, `*+N` (e.g. `vlen set *-vstart` = frame size).
			# Here `*` is the running csect offset, NOT a code address.
			oper = pl_oper; gsub(/[ \t]/, "", oper)
			v = off
			rest = substr(oper, 2)		# text after the leading '*'
			if (rest != "") {
				sign = substr(rest, 1, 1); rest = substr(rest, 2)
				if (rest ~ /^[0-9]/) rest = rest + 0
				else rest = (rest in csym) ? csym[rest] : 0
				v = (sign == "-") ? off - rest : off + rest
			}
			print pl_lbl "\tequ\t" v
			csym[pl_lbl] = v
		} else if (vb[i] !~ /^[ \t]*$/) {
			print vb[i]			# equ <const> / other directive: pass through
		}
	}
}

# ---- flush one buffered vsect region -----------------------------------------
# vb[1..vbn] are the region's source lines; k is the region index.
function flush_region(k,   i, hasinit, m, ops, j, il, rl, eqn, op, line) {
	# does the region initialise anything (vs. pure rmb/rzb reservation)?
	hasinit = 0
	for (i = 1; i <= vbn; i++) {
		parse_line(vb[i])
		if (pl_mnem == "fcb" || pl_mnem == "fdb" || pl_mnem == "fcc" \
		    || pl_mnem == "fcn" || pl_mnem == "fqb") { hasinit = 1; break }
	}

	if (!hasinit) {
		# uninitialised: reserve straight into bss (crt0 zeroes it, so an
		# explicit rzb is equivalent to a plain rmb here).
		print "\tsection\tbss"
		for (i = 1; i <= vbn; i++) {
			parse_line(vb[i])
			if (pl_mnem == "nam" || pl_mnem == "ttl") continue
			if (pl_mnem == "rzb") pl_mnem = "rmb"
			line = (pl_lbl != "") ? pl_lbl ":" : ""
			if (pl_mnem != "") line = line "\t" pl_mnem (pl_oper != "" ? "\t" pl_oper : "")
			print line
		}
		print "\tsection\tcode"
		return
	}

	# initialised: emit the loaded image, then bss reservation, equates, the
	# copy record and any pointer relocations.
	il = 0			# image item counter (per region)
	eqn = 0			# pending equate count
	print "\tsection\tdata"
	print pfx ".img." k ":"
	for (i = 1; i <= vbn; i++) {
		parse_line(vb[i])
		if (pl_mnem == "nam" || pl_mnem == "ttl") continue	# no image bytes
		if (pl_lbl != "") {
			# image label for this symbol, and an equate giving its
			# data-area offset (bss base + position within the image)
			print pfx ".il." k "." il ":"
			eq[eqn++] = pl_lbl "\tequ\t" pfx ".bss." k "+(" pfx ".il." k "." il "-" pfx ".img." k ")"
			il++
		}
		if (pl_mnem == "") continue

		if (pl_mnem == "fdb" || pl_mnem == "fqb") {
			m = split_ops(pl_oper, ops)
			for (j = 1; j <= m; j++) {
				op = ops[j]
				if (is_symbol(op)) {
					rl = pfx ".r." k "." nreloc
					print rl ":\t" pl_mnem "\t" op
					# data symbol -> +Y (ireloc); else (code label,
					# string literal, function) -> +module base (icreloc)
					if (op in isdata)
						drel[ndrel++] = pfx ".bss." k "+(" rl "-" pfx ".img." k ")"
					else
						crel[ncrel++] = pfx ".bss." k "+(" rl "-" pfx ".img." k ")"
					nreloc++
				} else {
					print "\t" pl_mnem "\t" op
				}
			}
		} else {
			# fcb / fcc / fcn pass through.  rmb / rzb reserve space that
			# must appear as real zero bytes in the LOADED image (so the
			# copy record covers them and later image offsets stay aligned);
			# lwasm emits zero bytes with zmb, whereas rmb would leave a gap
			# and rzb is not an lwasm opcode at all.
			if (pl_mnem == "rmb" || pl_mnem == "rzb") pl_mnem = "zmb"
			print "\t" pl_mnem (pl_oper != "" ? "\t" pl_oper : "")
		}
	}
	print pfx ".end." k ":"

	print "\tsection\tbss"
	print pfx ".bss." k ":\trmb\t" pfx ".end." k "-" pfx ".img." k

	for (i = 0; i < eqn; i++) print eq[i]	# label in column 0

	print "\tsection\tirec"
	print "\tfdb\t" pfx ".img." k
	print "\tfdb\t" pfx ".end." k
	print "\tfdb\t" pfx ".bss." k

	print "\tsection\tcode"
}

END {
	# ---- pass 1: defined / referenced symbols, data-symbol set, module prefix
	depth = 0			# >0 while inside a vsect
	for (i = 0; i < n; i++) {
		s = raw[i]
		if (s ~ /^[ \t]*\*/ || s ~ /^[ \t]*$/) continue	# RMA comment / blank
		if (s ~ /^[ \t]+psect[ \t]/) {
			if (!gotpfx) {
				pfx = s
				sub(/^[ \t]+psect[ \t]+/, "", pfx)
				sub(/[ \t,].*$/, "", pfx)
				gsub(/[^A-Za-z0-9_]/, "_", pfx)
				if (pfx == "") pfx = "anon"
				gotpfx = 1
			}
			continue
		}
		if (s ~ /^[ \t]+vsect([ \t]|$)/) { depth++; continue }
		if (s ~ /^[ \t]+endsect([ \t]|$)/) { if (depth > 0) depth--; continue }

		lbl = tok_label(s)
		if (lbl != "") {
			defined[lbl] = 1
			if (depth > 0) isdata[lbl] = 1
		}
		rest = s
		if (s ~ /^[A-Za-z_.]/) sub(/^[^ \t:]+:?/, "", rest)
		sub(/^[ \t]+/, "", rest)
		mnem = rest; sub(/[ \t].*$/, "", mnem)
		if (mnem == "nam" || mnem == "ttl") continue
		oper = rest; sub(/^[^ \t]+/, "", oper)
		collect_refs(oper)
	}

	# ---- emit EXTERN / EXPORT header ----
	for (t in referenced)
		if (!(t in defined)) print "\textern\t" t
	for (t in defined)
		if (t !~ /^_[0-9]/ && t !~ /^_O[0-9]/ && t != "")
			print "\texport\t" t

	# ---- pass 2: translate body ----
	nreloc = 0; ndrel = 0; ncrel = 0; region = 0
	i = 0
	while (i < n) {
		s = raw[i]
		if (s ~ /^[ \t]+psect[ \t]/) { print "\tsection\tcode"; i++; continue }
		if (s ~ /^[ \t]+vsect([ \t]|$)/) {
			# buffer this vsect's body up to its matching endsect
			i++; vbn = 0; d = 1
			while (i < n && d > 0) {
				t = raw[i]
				if (t ~ /^[ \t]+vsect([ \t]|$)/) d++
				else if (t ~ /^[ \t]+endsect([ \t]|$)/) { d--; if (d == 0) break }
				if (d > 0) vb[++vbn] = t
				i++
			}
			i++				# consume the closing endsect
			flush_region(region++)
			continue
		}
		if (s ~ /^[ \t]+csect([ \t]|$)/) {
			# RMA csect: a frame/DP work area whose `name rmb N` entries are
			# OFFSETS (the storage is reserved on the stack at run time and
			# accessed as name,u).  Buffer to its matching endsect and emit
			# offset equates - no section is opened.
			i++; vbn = 0; d = 1
			while (i < n && d > 0) {
				t = raw[i]
				if (t ~ /^[ \t]+csect([ \t]|$)/) d++
				else if (t ~ /^[ \t]+endsect([ \t]|$)/) { d--; if (d == 0) break }
				if (d > 0) vb[++vbn] = t
				i++
			}
			i++				# consume the closing endsect
			flush_csect()
			continue
		}
		if (s ~ /^[ \t]+endsect([ \t]|$)/) { print "\tendsection"; i++; continue }
		if (s ~ /^[ \t]+(nam|ttl)([ \t]|$)/) { sub(/^[ \t]+/, "", s); print "* " s; i++; continue }
		print s; i++
	}

	# emit the relocation tables collected during pass 2
	if (ndrel > 0) {
		print "\tsection\tireloc"
		for (i = 0; i < ndrel; i++) print "\tfdb\t" drel[i]
	}
	if (ncrel > 0) {
		print "\tsection\ticreloc"
		for (i = 0; i < ncrel; i++) print "\tfdb\t" crel[i]
	}
}
