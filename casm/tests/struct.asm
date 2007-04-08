	lib	casm.inc

	setdp	$ff
	org	$e00

anim	struct
cycle	fcb
cycle1	fcb
cycle2	fcb
cycle3	fcb
gotgold	fcb
doauto	fcb
blu	fdb
bu	fcb
bru	fcb
bl	fcb
bc	fcb
br	fcb
bld	fcb
bd	fcb
brd	fcb
freeze	fcb
sprite	fcb
mstat	fcb	4
sizex	fcb
	ends


object	struct
x	fcb
y	fcb
xx	fcb
yy	fcb	*
array	fcb	6
anim	anim	2	
	ends
*
* Macro to convert a MAP block ID to an MMU block and memory 
*
;test	namespace
;test2	namespace
;
player	object	5
;	endnamespace
;	endnamespace
;

;	ldx	#test::test2::player.anim[4].mstat[2]

exec	pshs	d,x,y,u,cc
	orcc	#$50

*
* Offset into structure
*
;	ldx	#object			$00
;	ldx	#object.x		$00
;	ldx	#object.y		$01
;	ldx	#object.xx		$02
;	lda	#object.yy		$03
	
	lda	object			$00
	lda	object.x		$00
	lda	object.y		$01
	lda	object.xx		$02
	lda	object.yy		$03
	
	lda	object.array		$04
	lda	object.array[0]		$04
	lda	object.array[1]		$05
	lda	object.array[2]		$06
	lda	object.array[3]		$07
	lda	object.array[4]		$08
	lda	object.array[5]		$09
	lda	object.anim.cycle	$0a
	lda	object.anim.cycle1	$0b
	lda	object.anim.cycle2	$0c
	lda	object.anim.cycle3	$0d
	lda	object.anim.gotgold	$0e
	lda	object.anim.doauto	$0f
	lda	object.anim.blu		$10	fdb
	lda	object.anim.bu		$12
	lda	object.anim.bru		$13
	lda	object.anim.bl		$14
	lda	object.anim.bc		$15
	lda	object.anim.br		$16
	lda	object.anim.bld		$17
	lda	object.anim.bd		$18
	lda	object.anim.brd		$19
	lda	object.anim.freeze	$1a
	lda	object.anim.sprite	$1b
	lda	object.anim.mstat	$1c	4
	lda	object.anim.mstat[0]	$1c	4
	lda	object.anim.mstat[1]	$1d	4
	lda	object.anim.mstat[2]	$1e	4
	lda	object.anim.mstat[3]	$1f	4
	lda	object.anim.sizex	$20
	
	lda	object.anim[0].cycle	$0a
	lda	object.anim[0].cycle1	$0b
	lda	object.anim[0].cycle2	$0c
	lda	object.anim[0].cycle3	$0d
	lda	object.anim[0].gotgold	$0e
	lda	object.anim[0].doauto	$0f
	lda	object.anim[0].blu	$10	fdb
	lda	object.anim[0].bu	$12
	lda	object.anim[0].bru	$13
	lda	object.anim[0].bl	$14
	lda	object.anim[0].bc	$15
	lda	object.anim[0].br	$16
	lda	object.anim[0].bld	$17
	lda	object.anim[0].bd	$18
	lda	object.anim[0].brd	$19
	lda	object.anim[0].freeze	$1a
	lda	object.anim[0].sprite	$1b
	lda	object.anim[0].mstat	$1c
	lda	object.anim[0].mstat[0]	$1c
	lda	object.anim[0].mstat[1]	$1d
	lda	object.anim[0].mstat[2]	$1e
	lda	object.anim[0].mstat[3]	$1f
	lda	object.anim[0].sizex	$20
	
	lda	object.anim[1].cycle	$21
	lda	object.anim[1].cycle1	$22
	lda	object.anim[1].cycle2	$23
	lda	object.anim[1].cycle3	$24
	lda	object.anim[1].gotgold	$25
	lda	object.anim[1].doauto	$26
	lda	object.anim[1].blu	$27	fdb
	lda	object.anim[1].bu	$28
	lda	object.anim[1].bru	$29
	lda	object.anim[1].bl	$2a
	lda	object.anim[1].bc	$2b
	lda	object.anim[1].br	$2c
	lda	object.anim[1].bld	$2d
	lda	object.anim[1].bd	$2e
	lda	object.anim[1].brd	$2f
	lda	object.anim[1].freeze	$30
	lda	object.anim[1].sprite	$31
	lda	object.anim[1].mstat	$32
	lda	object.anim[1].mstat[0]	$33
	lda	object.anim[1].mstat[1]	$34
	lda	object.anim[1].mstat[2]	$35
	lda	object.anim[1].mstat[3]	$36
	lda	object.anim[1].sizex	$37
	
	
*
* Direct offset into declared object
*
	ldx	#player[1]		4
	ldx	#player			0
	ldx	#player.y		1
	ldx	player.y		1
	ldx	player[1].y		5
	ldd	#player[1].array[1]	6
	ldx	player[1].y,x		5
	ldd	#player[1].array[1],x	6

*
* Offset into defined object
*
*	ldx	#player			e00
*	ldx	#player.y		e01
*	lda	player.y		e00
*	ldx	#player[1]		e04
*	ldx	#player[1].y[1]		e06
*	lda	player[1].y		e05
*	ldx	player[1].y[1],x	e06
*	lda	player.xx.mastat[2],x

	puls	d,x,y,u,cc,pc

