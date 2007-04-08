;
; This file tests the local label management within CASM. In order for local
; labels to be assigned correctly it must take into account usage and count
; increments for empty lines
;
;
	ifp1



@a	
	else
	
@a	
	endif
	bra	@a
