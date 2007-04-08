	lib	casm.inc

;
;	Generic regression test for namespaces
;
;
	setdp	$ff
	org	$e00


	jsr	base


mem	namespace

malloc	nop
	jsr	base
	rts

free	nop
	jsr	base
	rts
	
	endnamespace

string	namespace

strlen	nop
	jsr	base
	rts
	
strcat	nop
	jsr	strlen
	jsr	base
	rts

newstr	nop
	jsr	:mem:malloc
	jsr	base
	rts

	endns



	jsr	string:strlen
	jsr	string:strcat
	jsr	string:newstr
	jsr	mem:malloc
	jsr	mem:free
	
	
base	nop
	rts
	
	
	end
