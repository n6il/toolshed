; putstr.s - write a NUL-terminated string to OS-9 standard output.
; c3 calling convention: single arg (char *) in D; Y is the reserved data base
; and MUST be preserved across the call.
		section	code
		export	putstr
PthSO		equ	1		; standard output path number
IWrite		equ	$8A		; OS-9 I$Write
putstr:
		pshs	u,y		; preserve frame ptr (U) and c3 data base (Y)
		tfr	d,u		; U = string start (scratch, restored on exit)
		tfr	d,x		; X = scan pointer
		ldy	#0		; Y = byte count
psl:		lda	,x+
		beq	psd
		leay	1,y
		bra	psl
psd:		tfr	u,x		; X = string start
		lda	#PthSO		; A = stdout path
		os9	IWrite		; write Y bytes at X
		puls	y,u,pc		; restore data base + frame, return
