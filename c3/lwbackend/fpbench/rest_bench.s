* rest_bench.s - bench _dneg, _itod, _dtoi, _dcmpr against MC6839 on usim09batch.

ACIAS equ $ffc4
ACIAD equ $ffc5
HALT  equ $ffcc
TDRE  equ $02

_flacc equ 0
_fpcb  equ 8
_fpreg equ 16
dbase  equ $6000

 org $0100
start
 lds #$7f00
 ldy #dbase
 ldd #rom+$3d
 std _fpreg,y
 lda #$20
 sta _fpcb,y
 clr _fpcb+1,y
 clr _fpcb+2,y
 clr _fpcb+3,y
 clr _fpcb+4,y
 clr _fpcb+5,y

* _dneg(5.0) -> -5.0 = C014000000000000
 lda #'N
 lbsr emit1
 ldx #c5
 lbsr _dneg
 lbsr showfla

* _itod(7) -> 7.0 = 401C000000000000
 lda #'I
 lbsr emit1
 ldd #7
 lbsr _itod
 lbsr showfla

* _itod(-3) -> -3.0 = C008000000000000
 lda #'i
 lbsr emit1
 ldd #-3
 lbsr _itod
 lbsr showfla

* _dtoi(5.0) -> 5 = 0005
 lda #'X
 lbsr emit1
 ldx #c5
 lbsr _dtoi
 lbsr showd

* _dtoi(-3.0) -> FFFD
 lda #'x
 lbsr emit1
 ldx #cm3
 lbsr _dtoi
 lbsr showd

* compares: 2?3 -> L, 3?3 -> E, 5?3 -> G
 lda #'C
 lbsr emit1
 ldx #c2
 lbsr _dstack
 ldx #c3
 lbsr _dcmpr
 lbsr cresult
 ldx #c3
 lbsr _dstack
 ldx #c3
 lbsr _dcmpr
 lbsr cresult
 ldx #c5
 lbsr _dstack
 ldx #c3
 lbsr _dcmpr
 lbsr cresult
 lda #$0d
 lbsr putc

 sta HALT
spin bra spin

cresult blt cL
 beq cE
 lda #'G
 lbra putc
cL lda #'L
 lbra putc
cE lda #'E
 lbra putc

emit1 lbsr putc
 lda #':
 lbra putc

showfla leax _flacc,y
 ldb #8
sf1 pshs b,x
 lda ,x
 lbsr puthex
 puls b,x
 leax 1,x
 decb
 bne sf1
 lda #$0d
 lbra putc

showd pshs b
 lbsr puthex
 puls a
 lbsr puthex
 lda #$0d
 lbra putc

_dstack leas -8,s
 ldd 8,s
 std ,s
 ldd ,x
 std 2,s
 ldd 2,x
 std 4,s
 ldd 4,x
 std 6,s
 ldd 6,x
 std 8,s
 rts

* _dneg: X->double, result->_flacc, x->_flacc (monadic FNEG $20)
_dneg pshs x
 leax _flacc,y
 pshs x
 leax _fpcb,y
 pshs x
 ldx _fpreg,y
 pshs x
 pshs y
 ldy 8,s
 ldx 6,s
 ldd 4,s
 jsr [2,s]
 fcb $20
 ldy ,s
 ldx 6,s
 leas 10,s
 rts

* _itod: D=int16, result->_flacc, x->_flacc (FFLTS $24)
_itod pshs d
 leax _flacc,y
 pshs x
 leax _fpcb,y
 pshs x
 ldx _fpreg,y
 pshs x
 pshs y
 leay 8,s
 ldx 6,s
 ldd 4,s
 jsr [2,s]
 fcb $24
 ldy ,s
 ldx 6,s
 leas 10,s
 rts

* _dtoi: X->double, returns int16 in D (FFIXS $16)
_dtoi pshs x
 leax _flacc,y
 pshs x
 leax _fpcb,y
 pshs x
 ldx _fpreg,y
 pshs x
 pshs y
 ldy 8,s
 ldx 6,s
 ldd 4,s
 jsr [2,s]
 fcb $16
 ldy ,s
 ldd _flacc,y
 leas 10,s
 rts

* _dcmpr: op1 on stack, X->op2, result in CC (signed), X preserved, op1 cleaned
_dcmpr pshs x
 leax _fpcb,y
 pshs x
 ldx _fpreg,y
 pshs x
 pshs y
 leau 10,s
 ldy 6,s
 ldd 4,s
 ldx #$0011
 jsr [2,s]
 fcb $8a
 tfr cc,a
 ldy ,s
 bita #$04
 bne ceq
 bita #$01
 bne cgt
 bita #$08
 bne clt
cgt ldb #1
 bra csav
clt ldb #$ff
 bra csav
ceq clrb
csav stb _flacc,y
 ldx 6,s
 ldd 8,s
 std 16,s
 leas 16,s
 lda _flacc,y
 rts

puthex pshs a
 lsra
 lsra
 lsra
 lsra
 bsr nyb
 puls a
 anda #$0f
nyb anda #$0f
 adda #'0
 cmpa #'9
 bls putc
 adda #7
putc pshs a
pcw lda ACIAS
 bita #TDRE
 beq pcw
 puls a
 sta ACIAD
 rts

c2  fcb $40,$00,$00,$00,$00,$00,$00,$00
c3  fcb $40,$08,$00,$00,$00,$00,$00,$00
c5  fcb $40,$14,$00,$00,$00,$00,$00,$00
cm3 fcb $c0,$08,$00,$00,$00,$00,$00,$00

rom includebin "Float09.bin"

 org $fffe
 fdb start
