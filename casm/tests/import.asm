


	.import		test1
	.extern		test2
	.extrn		test3
	extern		test4
	
	
	org		$4000
	ldx		#test1
	ldx		#test2
	ldx		#test3
	ldx		#test4
	
