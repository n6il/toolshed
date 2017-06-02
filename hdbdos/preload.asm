* This preloader code should be 0x30 bytes long (+ 5 bytes preamble)
* The preloader is loaded at 0x4fd0 and the appended hdbdos is thus
* loaded at 0x5000.
* When run, the preloader copies the code from 0x5000 to 0xc000 and
* executes it there.

* DECB binary file preamble

	fcb     $00		Preamble flag
	fdb	$2030		Length of data block
	fdb	$4FD0		Load address

* Entry point from Basic

	org	$4fd0		This code covers $4fd0-$4fff
	orcc	#$50		Disable interrupts
	lda	>$fffe		Check RESET vector
	cmpa	#$8c		Points to CoCo3 reset code?
	beq	reloc		If yes, skip ROM copy

* Copy BASIC ROM to RAM

	ldx	#$8000		Start of ROM
copyrom	sta	>$ffde		Switch to ROM page
	lda	,x
	sta	>$ffdf		Switch to RAM page
	sta	,x+
	cmpx	#$c000		End of ROM
	bne	copyrom

* Relocate HDBDOS

reloc	ldx	#$5000		Copy from loaded address
	ldu	#$c000		to ROMPAK area
copyhdb	lda	,x+
	sta	,u+
	cmpx	#$7000
	bne	copyhdb
	jsr	>$a928		Color Basic CLEAR SCREEN
	jmp	>$c002		Start HDBDOS
