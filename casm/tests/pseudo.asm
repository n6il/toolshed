	lib	casm.inc

;
; Quick test to make sure the pseudo ops are picked up
; does not include conditionals
;
;


	ttl	sometitlehere
	title	anothertitlehere
	nam	hello
	name	hello
	opt	l
	
	
	org	$1000
	
	pag
	page
	spc	10
	
	ifdef	__CASM_DEBUG__
	printdp
	pushdp
	setdp	$11
	printdp
	popdp
	printdp
	else
	pushdp
	setdp	$11
	popdp
	endif
	
	
	


set1	equ	$00
set2	=	$00
set3	set	$00


type1	macro
	endm
	
type2	struct
	fcb	1
	ends
	
type3	struct
	fcb	1
type31	union
a	fcb	1
b	fdb	1
	endunion
	endstruct
	
	

	
	.bss
	.code
	.data
	.import		somelabel
	.extern		someotherlabel
	.export		prog1
	export		prog2


	align	1
	even
	odd
	
	bsz	100
	fcb	$00
	fcc	"hello"
	fcs	"hello"
	fcn	"hello"
	fcr	"hello"
	fcz	"hello"
	fdb	5
	fill	100,$44
	fqb	5
	fzb	5
	fzd	5
	fzq	5
	rmb	5
	rmd	5
	rmq	5
	rzb	5
	rzd	5
	rzq	5
	zmb	5
	zmd	5
	zmq	5
	
	includebin	a.bat
	raw		a.bat
	use		union.asm
	include		phasing.asm
	lib		expr.asm
	
		
ns1	namespace
	endns
	
ns2	namespace
	endnamespace
	
	
	mod	dd
	os9	#$00

	psect
	endsect
	
	vsect
	endsect
	
