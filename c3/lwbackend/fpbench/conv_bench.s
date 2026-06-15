* conv_bench.s - bench the MC6839 conversion wrappers: _ltod _dtol _dtof _ftod _utod
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

 lda #'L                _ltod(65536) -> 40F0000000000000
 lbsr emit1
 ldx #long64k
 lbsr _ltod
 ldb #8
 lbsr shown
 lda #'T                _dtol(65536.0) -> 00010000
 lbsr emit1
 ldx #d64k
 lbsr _dtol
 ldb #4
 lbsr shown
 lda #'F                _dtof(5.0) -> 40A00000
 lbsr emit1
 ldx #d5
 lbsr _dtof
 ldb #4
 lbsr shown
 lda #'D                _ftod(5.0f) -> 4014000000000000
 lbsr emit1
 ldx #f5
 lbsr _ftod
 ldb #8
 lbsr shown
 lda #'U                _utod(32768) -> 40E0000000000000
 lbsr emit1
 ldd #$8000
 lbsr _utod
 ldb #8
 lbsr shown
 sta HALT
spin bra spin

emit1 lbsr putc
 lda #':
 lbra putc
shown leax _flacc,y     B = byte count
sn1 pshs b,x
 lda ,x
 lbsr puthex
 puls b,x
 leax 1,x
 decb
 bne sn1
 lda #$0d
 lbra putc

_ltod pshs x
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
 fcb $26
 ldy ,s
 ldx 6,s
 leas 10,s
 rts

_dtol pshs x
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
 fcb $18
 ldy ,s
 ldx 6,s
 leas 10,s
 rts

_dtof pshs x
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
 ldu #$0010
 jsr [2,s]
 fcb $9a
 ldy ,s
 ldx 6,s
 leas 10,s
 rts

_ftod pshs x
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
 ldu #$0001
 jsr [2,s]
 fcb $9a
 ldy ,s
 ldx 6,s
 leas 10,s
 rts

_utod pshs d
 ldd #0
 pshs d
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
 fcb $26
 ldy ,s
 ldx 6,s
 leas 12,s
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

long64k fcb $00,$01,$00,$00
d64k    fcb $40,$f0,$00,$00,$00,$00,$00,$00
d5      fcb $40,$14,$00,$00,$00,$00,$00,$00
f5      fcb $40,$a0,$00,$00

rom includebin "Float09.bin"
 org $fffe
 fdb start
