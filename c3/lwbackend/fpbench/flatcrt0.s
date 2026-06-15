* flatcrt0.s - bare-metal crt0 stand-in for the usim09batch FP smoke test.
* Mimics what the real crt0/fpinit do (set Y=data base, _fpreg from FPO9, _fpcb)
* but with no OS-9: embeds FPO9 and points _fpreg at rom+$3D directly.
 export start,_flacc,_fpcb,_fpreg,_fpmod,_fpstak,_stkcheck,_rpterr
 import main,c_,i_
ACIAS equ $ffc4
ACIAD equ $ffc5
HALT  equ $ffcc
TDRE  equ $02
 section code
start
 lds #$7f00
 ldy #$6000           Y = data base
 ldd #rom+$3d
 std _fpreg,y         _fpreg = MC6839 FPREG entry
 lda #$29
 sta _fpcb,y
 clr _fpcb+1,y
 clr _fpcb+2,y
 clr _fpcb+3,y
 clr _fpcb+4,y
 clr _fpcb+5,y
 lbsr main
 lda #'C
 lbsr emit1
 leax c_,y
 ldb #8
 lbsr dumpn
 lda #'I
 lbsr emit1
 leax i_,y
 ldb #2
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
_stkcheck rts
_rpterr rts
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
rom includebin "Float09.bin"
 section bss
_flacc rmb 8
_fpmod rmb 2
_fpreg rmb 2
_fpstak rmb 2
_fpcb rmb 6
 section vectors
 fdb start
