	lib	casm.inc


	setdp	$00
	
	org	$00
	
var1	fdb	$0000
var2	fdb	$0000

	org	$1000

var3	fdb	$0000
var4	fdb	$0000


	org	$2000
	
	ldd	var1		* Direct access
	ldd	var2		* Direct access
	ldd	var3		* Extended access
	ldd	var4		* Extended access
	ldd	var5		* forced to extended addressing by assembler (forward reference)
	ldd	<var6		* Explicit direct addressing
	ldd	>var2		* Explicit extended addressing
;	ldd	<var3		* Explicit direct addressing (should fail)
	
	
	org	$00
var5	fdb	$0000
var6	fdb	$0000

