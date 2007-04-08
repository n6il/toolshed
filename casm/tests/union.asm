	lib	casm.inc

	setdp	$ff
	org	$e00
;
;	Test unions
;
;

one	struct
a	fdb		* +0
b	fdb		* +2
c	fcb		* +4
	endstruct
	
two	struct
d	fdb		* +0
e	fcb		* +2
	endstruct
	
three	struct
f	fdb		* 2
x	union
g	one		* 5
h	two		* 3
	endunion	*---> 8
	endstruct


test	three
	
	fdb	sizeof(one)
	fdb	sizeof(two)
	fdb	sizeof(three)
; not supported yet
;	fdb	sizeof(test.x)
;	fdb	sizeof(test.x.g)
;	fdb	sizeof(test.x.h)


	lda	test.x.g.c	; 6
	lda	test.x.h.e	; 4
