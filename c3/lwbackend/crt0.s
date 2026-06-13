; crt0.s - minimal C startup for the c3 lwasm/lwlink (OS-9 module) backend.
;
; This replaces the Microware cstart.r when linking c3 output with lwlink
; instead of rlink.  lwlink resolves all references at link time, so the
; Microware cstart's runtime relocation/init-image machinery is neither
; present nor needed - this is a fresh, minimal startup for that model.
;
; OS-9 program-module entry register convention:
;     U = lowest address of the allocated data area (data base)
;     Y = highest address of the data area (top of stack)
;     D = parameter-area size,  X = parameter pointer
; The c3 code generator reserves Y as the global/static data base
; ("C progs assume data & bss offset from y", per the original cstart).

		section	code
		export	__start
		extern	main

__start:
		leay	0,u		; Y = global/static data base (c3 ABI)
; (initialised-data copy and bss clear go here once the backend emits an
;  init-data image; OS-9 F$Fork does not guarantee a zeroed data area.)
		lbsr	main		; run the program; return value in D
		clrb			; exit status 0  (B = status)
		os9	$06		; F$Exit

; Stack-overflow check.  The compiled prologue does its own `leas` to make
; room for locals; _stkcheck only validates there is space.  Stubbed as a
; no-op for now (no overflow detection).
		export	_stkcheck
_stkcheck:
		rts

; Divide-by-zero / range-error reporter referenced by the integer-divide and
; long-math helpers.  The Microware version signals the process; stubbed here
; as a no-op so those helpers link without pulling in the OS-9 syscall defs.
		export	_rpterr
_rpterr:
		rts

; Shared runtime globals, accessed by the helpers as "name,y" (Y = data base).
; They live in the bss data area and are exported so the library resolves them.
		section	bss
		export	errno
		export	_flacc
errno:		rmb	2		; global errno
_flacc:		rmb	8		; long / floating-point accumulator

; The OS-9 module's data area must cover globals (bss) AND the runtime
; stack, since lwlink sizes the data area from the bss sections only.
; Reserve a default stack here; grow it via the driver if a program needs
; more (the Microware build folded this into the psect "Stk" parameter).
STACKSIZE	equ	1024
		rmb	STACKSIZE
