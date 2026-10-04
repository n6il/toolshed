* dyad_bench.s - bench the c3-ABI MC6839 dyadic wrappers on usim09batch.
* c3 convention: op1 pushed on stack (via _dstack), X->op2, Y=data base;
* result -> _flacc, X returns ->_flacc, op1 cleaned off the stack.
* Tests: 2+3=5, 7-3=4, 3*4=12, 12/3=4.

ACIAS equ $ffc4
ACIAD equ $ffc5
HALT  equ $ffcc
TDRE  equ $02

* data-area offsets (Y = data base), mirroring crt0's bss layout
_flacc equ 0
_fpcb  equ 8
_fpreg equ 16
dbase  equ $6000

* dyadic wrapper body, \1 = MC6839 opcode.  Entry: X->op2, op1 at 2,s, Y=databar
dyad macro
 pshs x              save op2 ptr
 leax _flacc,y
 pshs x              &_flacc (result)
 leax _fpcb,y
 pshs x              &_fpcb
 ldx _fpreg,y
 pshs x              FPREG entry
 pshs y              data base
 leau 12,s           U -> op1 (on stack)
 ldy 8,s             Y -> op2
 ldx 6,s             X -> _flacc (result buffer)
 ldd 4,s             D -> _fpcb
 jsr [2,s]           call MC6839 FPREG
 fcb \1              opcode
 ldy ,s              restore data base
 ldx 6,s             X -> _flacc (c3 return convention)
 ldd 10,s            return address
 std 18,s            move it below the (cleaned) op1
 leas 18,s           drop frame + op1
 rts
 endm

 org $0100
start
 lds #$7f00
 ldy #dbase
 ldd #rom+$3d        _fpreg = FPREG entry
 std _fpreg,y
 lda #$20            _fpcb CTL = double, nearest
 sta _fpcb,y
 clr _fpcb+1,y
 clr _fpcb+2,y
 clr _fpcb+3,y
 clr _fpcb+4,y
 clr _fpcb+5,y

 lda #'A
 bsr emit1
 ldx #c2
 lbsr _dstack
 ldx #c3
 lbsr _dadd          2 + 3
 lbsr showfla

 lda #'S
 bsr emit1
 ldx #c7
 lbsr _dstack
 ldx #c3
 lbsr _dsub          7 - 3
 lbsr showfla

 lda #'M
 bsr emit1
 ldx #c3
 lbsr _dstack
 ldx #c4
 lbsr _dmul          3 * 4
 lbsr showfla

 lda #'D
 bsr emit1
 ldx #c12
 lbsr _dstack
 ldx #c3
 lbsr _ddiv          12 / 3
 lbsr showfla

 sta HALT
spin bra spin

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

_dadd dyad $00
_dsub dyad $02
_dmul dyad $04
_ddiv dyad $06

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
c4  fcb $40,$10,$00,$00,$00,$00,$00,$00
c7  fcb $40,$1c,$00,$00,$00,$00,$00,$00
c12 fcb $40,$28,$00,$00,$00,$00,$00,$00

rom includebin "Float09.bin"

 org $fffe
 fdb start
