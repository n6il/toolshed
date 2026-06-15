* fadd_baseline.s - prove MC6839 FADD on usim09batch (no c3 wrapper).
* Embeds Float09.bin (FPO9), calls FPREG (rom+$3D) with the register ABI,
* dumps the 8-byte result as hex on the ACIA, then halts.
* Expect 2.0 + 3.0 = 5.0 = $4014000000000000.

ACIAS equ $ffc4
ACIAD equ $ffc5
HALT  equ $ffcc
TDRE  equ $02

 org $0100
start
 lds #$7f00
 ldu #arg1            U -> arg1
 ldy #arg2            Y -> arg2
 ldx #result          X -> result buffer
 ldd #fpcb            D -> FPCB
 lbsr fpreg           MC6839 register entry
 fcb $00              FADD
 ldx #result          dump result[0..7] as hex
 ldb #8
dloop pshs b,x
 lda ,x
 bsr puthex
 puls b,x
 leax 1,x
 decb
 bne dloop
 lda #$0d
 bsr putc
 sta HALT             halt the batch sim
spin bra spin

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

fpcb fcb $20,$00,$00,$00,$00,$00
arg1 fcb $40,$00,$00,$00,$00,$00,$00,$00
arg2 fcb $40,$08,$00,$00,$00,$00,$00,$00
result rmb 8

rom includebin "Float09.bin"
fpreg equ rom+$3d

 org $fffe
 fdb start
