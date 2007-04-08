GSCRN	equ	$400
	ldx	#GSCRN+(45*32)+14
	fcb	$22,2[$32],$2A,2[$26],$22,0,0
	
test	macr	var0,var1,var2
	\0		* Macro-80C style macro var
	{var1}		* CASM style macro var
\.AA	SET	\2	* Macro-80C style macro label
	endm
	test	nop,clra,1
	

	leax	-1,x
	leax	1,x

@a	bra	c@
@b	bra	@b
c@	bra	@a
