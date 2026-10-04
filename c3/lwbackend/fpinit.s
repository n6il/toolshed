	extern	_fpreg
	extern	_fpmod
	extern	_fpcb
	extern	_fpstak
	export	FPO9TY
	export	fpname
	export	fppath
	export	_fpinit
	export	fpfail
	export	fptdone
	export	_fpterm
	export	FPCBCTL
	export	fphave
* fpinit.a - link the MC6839 IEEE floating-point package (FPO9) at startup.
*
* crt0 calls _fpinit after the data area is zeroed / before main, and _fpterm
* after main returns.  This object is selected (over the fpdummy.a no-op) only
* when the program is known to use floating point - it is meant to live with the
* FP helper wrappers, so any FP operation pulls it in; a program that never
* touches FP gets fpdummy and so never links FPO9.
*
* MC6839 module:  name "FPO9", type/language $B1.  Its named entries sit at
* fixed offsets from the module base:  FPREG at +$3D, FPSTAK at +$3F.  We
* $00 it (it must be resident); if it is not yet loaded we $01 it from
* /DD/Modules.  The results live in crt0's data area (Y-relative):
*   _fpmod  = module base (for $02)
*   _fpreg  = base + $3D   _fpstak = base + $3F
*   _fpcb   = 5-byte control block, CTL=$29 (double | NRM | nearest | affine)
	section	code

FPO9TY  equ $b1                 FPO9 module type/language
FPCBCTL equ $29                 double precision, normalised, round-nearest, affine

_fpinit:
 leax fpname,pcr                X = module name "FPO9"
 lda #FPO9TY
 pshs y                         preserve the data base across the syscall
 os9 $00                     U = module base, or carry set + B = error
 puls y
 bcc fphave
 cmpb #$DD                    not resident yet?
 bne fpfail                     any other link error is fatal
 leax fppath,pcr                X = "/DD/Modules/FPO9"
 lda #FPO9TY
 pshs y
 os9 $01                     load it from disk, U = module base
 puls y
 bcs fpfail
fphave:
 stu _fpmod,y                   remember the base for $02
 leax $3d,u
 stx _fpreg,y                   FPREG  = base + $3D
 leax $3f,u
 stx _fpstak,y                  FPSTAK = base + $3F
 lda #FPCBCTL
 sta _fpcb,y                    control byte (must be nonzero)
 clr _fpcb+1,y                  ENB
 clr _fpcb+2,y                  ERR
 clr _fpcb+3,y                  SS
 clr _fpcb+4,y                  TRAPV
 rts

* FPO9 unavailable: leave _fpmod/_fpreg zero and return.  crt0/cstart call
* _fpinit for every program, so this must be non-fatal - a no-FP program must
* still run when FPO9 is absent.  (A program that *does* use FP will then fault
* on its first FP op; the fix is to install FPO9, see fppath.)
fpfail:
 ldd #0
 std _fpmod,y
 std _fpreg,y
 rts

* OS-9 name strings end with the high bit set on the last character
fpname fcc "FPO"
 fcb '9'+$80
fppath fcc "/DD/Modules/FPO"
 fcb '9'+$80


_fpterm:
 ldu _fpmod,y                   module base, 0 if FP was never linked
 beq fptdone
 pshs y
 os9 $02
 puls y
fptdone rts

	endsection
