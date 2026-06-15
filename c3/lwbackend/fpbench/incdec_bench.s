* incdec_bench.s - bench _dinc/_ddec (double, in-place +/-1.0) and _finc/_fdec (float).
ACIAS equ $ffc4
ACIAD equ $ffc5
HALT  equ $ffcc
TDRE  equ $02
_fpcb  equ 8
_fpreg equ 16
dbase  equ $6000

 org $0100
start
 lds #$7f00
 ldy #dbase
 ldd #rom+$3d
 std _fpreg,y
 lda #$29
 sta _fpcb,y
 clr _fpcb+1,y
 clr _fpcb+2,y
 clr _fpcb+3,y
 clr _fpcb+4,y
 clr _fpcb+5,y

 lda #'P                _dinc(5.0) -> 6.0 = 4018000000000000
 lbsr emit1
 ldx #d5
 lbsr _dinc
 leax d5,pcr
 ldb #8
 lbsr dumpn
 lda #'M                _ddec(5.0->6.0 now) -> back to 5.0 = 4014000000000000
 lbsr emit1
 ldx #d5
 lbsr _ddec
 leax d5,pcr
 ldb #8
 lbsr dumpn
 lda #'p                _finc(5.0f) -> 6.0f = 40C00000
 lbsr emit1
 ldx #f5
 lbsr _finc
 leax f5,pcr
 ldb #4
 lbsr dumpn
 lda #'m                _fdec(6.0f now) -> 5.0f = 40A00000
 lbsr emit1
 ldx #f5
 lbsr _fdec
 leax f5,pcr
 ldb #4
 lbsr dumpn
 sta HALT
spin bra spin

emit1 lbsr putc
 lda #':
 lbra putc
dumpn pshs b,x
 lda ,x
 lbsr puthex
 puls b,x
 leax 1,x
 decb
 bne dumpn
 lda #$0d
 lbra putc

_dinc pshs x
 pshs y
 ldx _fpreg,y
 pshs x
 leax _fpcb,y
 pshs x
 ldu 6,s
 leay dione,pcr
 ldx 6,s
 ldd ,s
 jsr [2,s]
 fcb $00
 ldy 4,s
 ldx 6,s
 leas 8,s
 rts
_ddec pshs x
 pshs y
 ldx _fpreg,y
 pshs x
 leax _fpcb,y
 pshs x
 ldu 6,s
 leay dione,pcr
 ldx 6,s
 ldd ,s
 jsr [2,s]
 fcb $02
 ldy 4,s
 ldx 6,s
 leas 8,s
 rts
_finc pshs x
 pshs y
 ldx _fpreg,y
 pshs x
 leax sfpcb,pcr
 pshs x
 ldu 6,s
 leay fione,pcr
 ldx 6,s
 ldd ,s
 jsr [2,s]
 fcb $00
 ldy 4,s
 ldx 6,s
 leas 8,s
 rts
_fdec pshs x
 pshs y
 ldx _fpreg,y
 pshs x
 leax sfpcb,pcr
 pshs x
 ldu 6,s
 leay fione,pcr
 ldx 6,s
 ldd ,s
 jsr [2,s]
 fcb $02
 ldy 4,s
 ldx 6,s
 leas 8,s
 rts
dione fcb $3f,$f0,$00,$00,$00,$00,$00,$00
sfpcb fcb $00,$00,$00,$00,$00,$00
fione fcb $3f,$80,$00,$00

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

d5 fcb $40,$14,$00,$00,$00,$00,$00,$00
f5 fcb $40,$a0,$00,$00

rom includebin "Float09.bin"
 org $fffe
 fdb start
