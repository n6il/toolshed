;
; This file tests the expression evaluation engine in CASM. The results
; are specific to the operation being tested and in some cases the result
; will be different depending on whether operator precedence has been
; enabled or not.
;
	ifdef	__CASM__
	opt	noprec			; Disable operator precedence
	endif


;-----------------------------------------------------------------------------
;
;	Basic evalutation check
;
	lda	-444,x



;-----------------------------------------------------------------------------
;
;	Basic operator check
;
	ldd	#5+2		; Result is 7
	ldd	#5-2		; Result is 3
	ldd	#5*2		; Result is 10
	ldd	#5/2		; Result is 2
	ldd	#5|8		; Result is 13
	ldd	#5&4		; Result is 4
	ldd	#11%3		; Result is 2
	ldd	#5^4		; Result is 1
	ldd	#5<6		; Result is 1
	ldd	#5<2		; Result is 0
	ldd	#5>2		; Result is 1
	ldd	#5>6		; Result is 0
	ldd	#5<>6		; Result is 1
	ldd	#5<>5		; Result is 0
	ldd	#5<=6		; Result is 1
	ldd	#5<=2		; Result is 0
	ldd	#5>=2		; Result is 1
	ldd	#5>=6		; Result is 0
	ldd	#5>=5		; Result is 1
	ldd	#5>=5		; Result is 1
	ldd	#5<<1		; Result is 10
	ldd	#5>>1		; Result is 2
	ldd	#1&&0		; Result is 0
	ldd	#3&&1		; result is 1
	ldd	#1||0		; result is 1
	ldd	#0||0		; result is 0

;-----------------------------------------------------------------------------
;
;	Basic sub-expressions
;
	ldd	#(2+2)-(1+1)		2
	ldd	#(24-(2*6))/3		4
	ldd	#24-((2*6)/3)		20

;-----------------------------------------------------------------------------
;
; Test Macro-80C style operator precedence
;
	ifdef	__CASM__
	
	ldd	#(90-37)!+(128-45)	; Result is $1e

	opt	prec			; Enable operator precedence

	ldd	#55-24+443-333*44	; Result is $C89E
	ldd	#24-(2*6)/3		; Result is $14

	opt	noprec			; Disable operator precedence
	
	ldd	#55-24+443-333*44	; Result is $183C
	ldd	#24-(2*6)/3		; Result is $04

	
	endif
