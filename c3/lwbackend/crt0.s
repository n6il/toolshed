; crt0.s - C startup for the c3 lwasm/lwlink (OS-9 module) backend.
;
; Replaces the Microware cstart.r when linking c3 output with lwlink.  Unlike
; the Microware ROF model (load-time relocation tables applied by the system),
; lwlink resolves everything at link time, so this startup does at run time the
; three jobs cstart used to get from the loader:
;
;   1. Zero the data area.   OS-9 F$Fork does NOT zero allocated memory, but C
;      requires uninitialised statics (and explicit `rzb`) to read as 0.  We
;      clear from the data base (U) up to the entry stack pointer (S); the
;      parameter area sits ABOVE S and is left intact.
;   2. Copy each initialised-data image into the data area.  The compiler
;      output (via c3lw.awk) keeps the initial values in the loaded module body
;      ("data" section) and a table of copy records in "irec"; each record is
;      src/end (module-relative) + dest (data-area offset).
;   3. Relocate pointer initialisers.  A `T *p = &thing;` stores only an offset
;      at link time; the real address needs a base added at run time:
;        - ireloc  words point into the data area  -> add Y (the data base)
;        - icreloc words point into the module body (string literals, function
;          addresses)                              -> add the module base
;
; OS-9 program-module entry register convention:
;     U = lowest address of the data area (data base)
;     Y = highest address of the data area (initial top of stack)
;     S = stack pointer (just below the parameter area)
;     D = parameter-area size,  X = parameter pointer
; The c3 code generator reserves Y as the global/static data base
; ("C progs assume data & bss offset from y", per the original cstart), so after
; setup Y holds the data base for the life of the program.

		section	code
		export	__start
		extern	main

; link-time bounds of the init tables (synthesised by lwlink per os9c3.ld)
		import	s_irec
		import	l_irec
		import	s_ireloc
		import	l_ireloc
		import	s_icreloc
		import	l_icreloc
		import	s_stack		; data-area offset where the stack begins,
					; i.e. the total size of all statics (bss)

__start:
		leay	0,u		; Y = global/static data base (c3 ABI)

; ---- 1. zero the statics  [U .. U+s_stack) ----------------------------------
; Bound the clear by the LINK-TIME static size (s_stack), NOT the entry stack
; pointer S: on Level 2 the entry S is not U+datasize, so an S-U count overruns
; the data area and clobbers memory (this is what cstart avoids by using its
; link-time `end` symbol).  The stack region above the statics needs no zeroing.
		ldd	#s_stack	; D = number of static bytes to clear
		beq	zdone
		leax	0,u		; X = data base
zloop:		clr	,x+
		subd	#1
		bne	zloop
zdone:

; ---- compute the module base, save base registers for the loops -------------
; module base = (run-time address of __start) - (link-time offset of __start)
		leax	__start,pcr	; X = absolute address of __start
		tfr	x,d
		subd	#__start	; D = module base
		std	_mbase,y	; (data area now zeroed, safe to use)
		tfr	y,d
		std	_ybase,y	; _ybase = absolute data base

; ---- 2. copy initialised-data images ----------------------------------------
		ldd	_mbase,y
		addd	#s_irec
		tfr	d,x		; X = absolute start of irec table
		ldd	_mbase,y
		addd	#s_irec
		addd	#l_irec
		pshs	d		; 0,s = absolute end of irec table
crec:		cmpx	0,s
		bhs	creced
		ldd	0,x		; record.src  (module-relative)
		addd	_mbase,y	; -> absolute source start
		std	_isrc,y
		ldd	2,x		; record.end  (module-relative)
		addd	_mbase,y	; -> absolute source end
		std	_iend,y
		ldd	4,x		; record.dest (data-area offset)
		leau	d,y		; U = absolute destination (Y + dest)
		pshs	x		; save irec walker
		ldx	_isrc,y
icopy:		cmpx	_iend,y
		bhs	icopyd
		lda	,x+
		sta	,u+
		bra	icopy
icopyd:		puls	x
		leax	6,x		; next record
		bra	crec
creced:		leas	2,s		; drop saved end

; ---- 3a. relocate data-area pointers  (add Y) -------------------------------
		ldd	_mbase,y
		addd	#s_ireloc
		tfr	d,x		; X = absolute start of ireloc table
		ldd	_mbase,y
		addd	#s_ireloc
		addd	#l_ireloc
		pshs	d		; 0,s = absolute end
dreloc:		cmpx	0,s
		bhs	drelocd
		ldd	,x++		; data-area offset of a pointer word
		leau	d,y		; U = address of that word
		ldd	,u		; stored value (target's data-area offset)
		addd	_ybase,y	; + data base -> absolute target
		std	,u
		bra	dreloc
drelocd:	leas	2,s

; ---- 3b. relocate module-body pointers  (add module base) -------------------
		ldd	_mbase,y
		addd	#s_icreloc
		tfr	d,x		; X = absolute start of icreloc table
		ldd	_mbase,y
		addd	#s_icreloc
		addd	#l_icreloc
		pshs	d		; 0,s = absolute end
creloc:		cmpx	0,s
		bhs	crelocd
		ldd	,x++		; data-area offset of a pointer word
		leau	d,y		; U = address of that word
		ldd	,u		; stored value (module-relative target)
		addd	_mbase,y	; + module base -> absolute target
		std	,u
		bra	creloc
crelocd:	leas	2,s

; ---- run the program --------------------------------------------------------
		lbsr	main		; return value in D
		clrb			; exit status 0  (B = status)
		os9	$06		; F$Exit

; Stack-overflow check.  The compiled prologue does its own `leas` to make
; room for locals; _stkcheck only validates there is space.  Stubbed as a
; no-op for now (no overflow detection).
		export	_stkcheck
_stkcheck:	rts

; Divide-by-zero / range-error reporter referenced by the integer-divide and
; long-math helpers.  The Microware version signals the process; stubbed here
; as a no-op so those helpers link without pulling in the OS-9 syscall defs.
		export	_rpterr
_rpterr:	rts

; Shared runtime globals, accessed by the helpers as "name,y" (Y = data base).
; They live in the bss data area and are exported so the library resolves them.
		section	bss
		export	errno
		export	_flacc
errno:		rmb	2		; global errno
_flacc:		rmb	8		; long / floating-point accumulator

; crt0 startup scratch (data-area, Y-relative; zeroed then written above)
_mbase:		rmb	2		; module base address (absolute)
_ybase:		rmb	2		; data base address (absolute, == Y)
_isrc:		rmb	2		; current image-copy source ptr
_iend:		rmb	2		; current image-copy source end

; Empty init tables so s_/l_ for these sections are always defined even when a
; program has no initialised data; lwlink concatenates same-named sections, so
; real records from compiled objects append after these zero-length anchors.
		section	irec
		section	ireloc
		section	icreloc

; Reserve the runtime stack in its own bss section, which os9c3.ld places LAST
; (top of the data area) above all statics; the stack descends from the top
; through this gap.  Grow it if a program needs more than the default.
		section	stack,bss
STACKSIZE	equ	1024
		rmb	STACKSIZE
