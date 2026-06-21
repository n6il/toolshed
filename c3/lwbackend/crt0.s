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
		extern	_fpinit		; link the MC6839 FP package (real fpinit.a,
					; or the fpdummy no-op if no FP is used)
		extern	_fpterm		; unlink it on exit

; link-time bounds of the init tables (synthesised by lwlink per os9c3.ld)
		import	s_irec
		import	l_irec
		import	s_ireloc
		import	l_ireloc
		import	s_icreloc
		import	l_icreloc
		import	s_stack		; data-area offset where the stack begins,
					; i.e. the total size of all statics (bss)
		import	l_directpage	; size of the direct-page section (must be <= 256)

MAXARGS		equ	32		; argv slot count (incl argv[0])

__start:
; ---- 1. zero the statics  [U .. U+s_stack) ----------------------------------
; Bound the clear by the LINK-TIME static size (s_stack), NOT the entry stack
; pointer S: on Level 2 the entry S is not U+datasize, so an S-U count overruns
; the data area and clobbers memory (this is what cstart avoids by using its
; link-time `end` symbol).  The stack region above the statics needs no zeroing.
; (Y still holds the entry top here - don't disturb it until captured below.)
		pshs	x		; save the OS-9 parameter pointer (entry X = params)
		ldd	#s_stack	; D = number of static bytes to clear
		beq	zdone
		leax	0,u		; X = data base
zloop:		clr	,x+
		subd	#1
		bne	zloop
zdone:
; OS-9 passes the top of the data area in Y at entry; capture it as `memend`
; (the high-water mark sbrk() grows the heap from, via F$Mem) BEFORE switching
; Y to the static data base for the rest of the program (the c3 ABI).
		tfr	y,d		; D = entry Y = real top of the data area
		leay	0,u		; Y = global/static data base
		std	memend,y
		puls	x		; recover the saved parameter pointer
		stx	_argp,y		; stash it (data area is now zeroed)

; ---- direct page: validate its size, then point DP at it --------------------
; The directpage section (os9c3.ld places it first) holds the `direct` globals at
; data-area offsets 0..255; the compiler reaches them with `<name` = (offset &
; 0xFF) on page DP.  Refuse to run if the page overflowed 256 bytes - otherwise
; `<name` would wrap to the wrong offset and silently corrupt memory.
		ldd	#l_directpage
		cmpd	#256
		bls	dpok
		leax	dperr,pcr	; complain to stderr and bail (this path exits,
		ldy	#dperrlen	; so clobbering Y - the data base - is fine)
		lda	#2		; stderr
		os9	$8C		; I$WritLn
		ldb	#1		; nonzero exit status
		os9	$06		; F$Exit
dpok:
; Y (== the data base) is page-aligned - OS-9 F$Fork data areas start on a 256-
; byte boundary - so DP = high(Y) makes `<name` resolve to Y+offset, the
; variable's real, zeroed home.  crt0's own globals are plain Y-relative
; (`name,y`, 3-byte) and are unaffected by DP.
		tfr	y,d		; D = data base; A = its high byte = the page
		tfr	a,dp		; set the direct page

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

; ---- C-library memory globals (memend was set at entry, above) --------------
; sbrk()/malloc() grow the heap ABOVE memend on demand via F$Mem, so the heap is
; not limited to the initial allocation.  _mtop/_stbot bound the in-area ibrk()
; allocator and the stack check.
		leax	s_stack,y	; end of statics = in-area heap base (ibrk)
		stx	_mtop,y		; current non-stack memory top
		sts	_stbot,y	; current stack bottom limit

; ---- parse the OS-9 parameter string into argc / argv -----------------------
; The shell passes the command line (CR-terminated) at _argp.  Split on spaces
; into argv[1..]; argv[0] is a placeholder (C code skips it).  Words are
; NUL-terminated in place.  Quotes are not handled - the c3 tools never need
; them.  Without this, arg-taking programs (c3prep et al.) get no argv and fall
; back to reading stdin, which blocks forever on the OS-9 console.
		leax	_argname,pcr	; argv[0] = placeholder program name
		stx	argv,y
		ldd	#1
		std	argc,y		; argc = 1 (argv[0])
		leau	argv+2,y	; U -> argv[1] slot
		ldx	_argp,y		; X -> parameter string
ap_sp:		lda	,x		; skip spaces
		cmpa	#$0d		; CR -> end of parameters
		beq	ap_done
		cmpa	#$20
		bne	ap_word
		leax	1,x
		bra	ap_sp
ap_word:	ldd	argc,y
		cmpd	#MAXARGS	; bound argv
		bhs	ap_done
		stx	,u++		; argv[argc] = start of this word
		addd	#1
		std	argc,y
ap_scan:	lda	,x		; scan to end of word
		cmpa	#$0d
		beq	ap_term0
		cmpa	#$20
		beq	ap_term
		leax	1,x
		bra	ap_scan
ap_term:	clr	,x+		; space -> NUL, step over it
		bra	ap_sp
ap_term0:	clr	,x		; CR -> NUL (last word)
ap_done:

; ---- run the program --------------------------------------------------------
		lbsr	_fpinit		; link the MC6839 FP module if this program uses FP
		leax	argv,y		; argv (2nd arg) on the stack
		pshs	x
		ldd	argc,y		; argc (1st arg) in D
		lbsr	main		; return value in D
		leas	2,s		; drop argv
		lbsr	_fpterm		; unlink the FP module (no-op if it was never linked)
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

; placeholder argv[0] (programs skip it; we don't get the real name from OS-9)
_argname:	fcc	"c3"
		fcb	0

; diagnostic for the direct-page overflow guard above (I$WritLn, CR-terminated)
dperr:		fcc	"c3: direct page exceeds 256 bytes"
		fcb	$0d
dperrlen	equ	*-dperr

; Shared runtime globals, accessed by the helpers as "name,y" (Y = data base).
; They live in the bss data area and are exported so the library resolves them.
		section	bss
		export	errno
		export	_flacc
errno:		rmb	2		; global errno
_flacc:		rmb	8		; long / floating-point accumulator

; MC6839 floating-point linkage.  _fpinit (fpinit.a) fills these when the
; program uses FP; the fpdummy stub leaves them zero.  The FP helper wrappers
; reach them as "name,y" (Y = data base), same as errno / _flacc.
		export	_fpmod
		export	_fpreg
		export	_fpstak
		export	_fpcb
_fpmod:		rmb	2		; FPO9 module base (0 = FP not linked)
_fpreg:		rmb	2		; MC6839 FPREG entry  (module base + $3D)
_fpstak:	rmb	2		; MC6839 FPSTAK entry (module base + $3F)
_fpcb:		rmb	5		; FP control block: CTL,ENB,ERR,SS,TRAPV

; crt0 startup scratch (data-area, Y-relative; zeroed then written above)
_mbase:		rmb	2		; module base address (absolute)
_ybase:		rmb	2		; data base address (absolute, == Y)
_isrc:		rmb	2		; current image-copy source ptr
_iend:		rmb	2		; current image-copy source end

; command-line arguments parsed from the OS-9 parameter string
_argp:		rmb	2		; saved parameter pointer (entry X)
argc:		rmb	2		; argument count
argv:		rmb	2*MAXARGS	; argument vector

; C-library memory-management globals (the heap allocator and stack check use
; these; normally set up by the Microware cstart, here by __start above).
		export	memend
		export	_mtop
		export	_stbot
memend:		rmb	2		; top of the data area
_mtop:		rmb	2		; current non-stack memory top (heap top)
_stbot:		rmb	2		; current stack bottom limit

; Empty init tables so s_/l_ for these sections are always defined even when a
; program has no initialised data; lwlink concatenates same-named sections, so
; real records from compiled objects append after these zero-length anchors.
		section	irec
		section	ireloc
		section	icreloc
; Likewise anchor the direct page so s_directpage/l_directpage always exist, even
; for a program that declares no `direct` globals (else the size-guard import
; fails to link).  Real direct vars from compiled objects append after this.
		section	directpage

; Reserve the runtime stack in its own bss section, which os9c3.ld places LAST
; (top of the data area) above all statics; the stack descends from the top
; through this gap.  This is the C call/locals stack only - the heap grows
; separately above memend via sbrk()/F$Mem - but a recursive program (e.g. the
; compiler) needs enough here; grow it if a program overflows.
		section	stack,bss
STACKSIZE	equ	4096
		rmb	STACKSIZE
