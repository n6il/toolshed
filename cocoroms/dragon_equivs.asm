* ROM entry point equivalents CoCo - Dragon 32
* Lxxxx is CoCo address, third column is Dragon equivalent
* The table is tab-separated

* Entry points without function name in ecb_equates.asm
* Coco	equ	Dragon	Comment
L813C	equ	$84F5	? EXBAS COMMAND INTERPRETATION LOOP
L8168	equ	$89b4  ?? (use SN ERROR for now) EXBAS SECONDARY COMMAND HANDLER
L8311	equ	$b6d4	?? CHECK EXBAS CLOAD HANDLER, then main loop
L8316	equ	$b682	?? DO A CSAVE, then main loop
L836C	equ	$994a   SYNTAX CHECK FOR COMMA - leaves value (2 bytes) on stack
L8748	equ	$9b7e   EVALUATE A STRING EXPRESSION
L880E	equ	$9c3e     Assign8BitB      evaluates octal number -> FP
L8955	equ	$9d46   GET REAL TIME CLOCK - into irq routine
L8C1B	equ	$a04e   (or $a04c ?) GET THE CURRENT INPUT CHARACTER -  DLOAD
L8CC6	equ	$a0ea	wait for keypress - flash cursor
L9539	equ	$a8c7	clear current graphics screen with B
L959A	equ	$a928	Set colours
L95AA	equ	$a938	Select Display mode (text z=1, graphics z=0)
L95AC	equ	$a93a     RESET SAM - reset VDU
L95FB	equ	$a989	Set VDG mode from A
L960F	equ	$a99d	Set VDG offset for graphics mode
L9616	equ	$a9a4	Set VDG colour set from $00c1
L962E	equ	$a9bc	SET START GRAPHIC PAGE, PMODE COMMAND
L9650	equ	$a9de	EVALUATE EXPRESSION - PMODE,x
L9653	equ	$a9e1	Set page to B
L9682	equ	$aa10	Select colour set 0 or 1 from B
L9695	equ	$aa23	Reserve HiRes Graphics RAM, move Basic if necessary
L96CB	equ	$aa66	PUT CORRECT ADDRESSES IN FIRST 2 BYTES OF EACH LINE - PCLEAR
L96EC	equ	$aa87	CONSTANT OFFSET OF $600 - init of EXBAS graphics vars
L975F	equ	$aafa	VEC22  CHECK FOR @ SIGN - PUT
L9AD7	equ	$ae9a	Play note from A
L9CB6	equ	$b051	Draw token dispatch address
L9FB5	equ	$b350	MULTIPLY (UNSIGNED) TWO 16 BIT NUMBERS TOGETHER
LA027	equ	$b3b4	Reset ISR
LA0B6	equ	$b400	Boot Basic
LA0E2	equ	$b449	WARM START FLAG - go to basic main loop
LA171	equ	$b505	GET A CHARACTER FROM CONSOLE IN
LA176	equ	$b50a	HOOK INTO RAM - console in, GET A CHAR FROM INPUT BUFFER
LA199	equ	$bbb5	Flashes cursor
LA1B1	equ	$b538	GETKEY
LA1C1	equ	$bbe5	%INCH% scans keyboard
LA282	equ	$b54a	%OUTCHR% to DEVNUM
LA30A	equ	$bcab	%OUTCH% to screen
LA323	equ	$bca0	Clear VDU line from cursor
LA35F	equ	$b595	SET PRINT PARAMETERS
LA37C	equ	$b5b2	SAVE THE PRINT PARAMETERS
LA39A	equ	$b5d0	inside routine that gets an input line for basic
LA2BF	equ	$bd1a	Send char A to printer
LA3ED	equ	$b623	VERIFY THAT THE FILE TYPE WAS 'INPUT
LA3FB	equ	$b631	'FILE NOT OPEN' ERROR
LA406	equ	$b63c	MAKE SURE SELECTED FILE IS AN OUTPUT FILE, TEST DEVICE NUMBER
LA426	equ	$b65c	CLOSE ALL FILES on cassette
LA429	equ	$b65f	CLOSE ALL FILES (skip ram vector)
LA42D	equ	$b663     Close file (on devnum)
LA511	equ	$b748	Read binary file from tape
LA469	equ	$b6a5	Write Basic program to cassette
LA545	equ	$b777	...EXEC
LA549	equ	$b77b	GO DO A BREAK CHECK
LA59A	equ	$b7cc	(or $bb97) MOVE (B) BYTES FROM (X) TO (U)
LA5A2	equ	$b7d4	GET DEVICE NUMBER
LA5A5	equ	$b7d7	EVALUATE AN EXPRESSION (# then DEVICE NUMBER)
LA5AE	equ	$b7e0	EVALUATE DEVICE NUMBER
LA5C7	equ	$b7f9	SYNTAX ERROR IF ANY MORE CHARS ON THIS LINE
LA5DA	equ	$b80c	BRANCH BACK TO BASIC'S EOF
LA5E4	equ	$b816	LINK BACK TO BASIC'S EOF STATEMENT
LA603	equ	$b835	get file name (open)
LA616	equ	$b848	BAD FILE MODE ERROR
LA61C	equ	$b84e	'FILE ALREADY OPEN' ERROR
LA61F	equ	$b851	DEVICE NUMBER ERROR
LA681	equ	$b8b3	Find File, search tape for matching filename
LA701	equ	$b933	Read first (filename) block from tape to buffer
LA70B	equ	$b93e	%BLKIN% from tape to buffer
LA749	equ	$bdad	%CBIN% input byte from tape to A
LA755	equ	$bda5	%BITIN% from tape to c
LA77C	equ	$bde7	%CSRDON% read leader
LA7CA	equ	$bdcf	%CASON%
LA7D1	equ	$bdd7	(not 100% compatible) In %CASON% WAIT A WHILE
LA7E5	equ	$b991	Write first block
LA7E9	equ	$bddc	CASOFF TURN OFF THE CASSETTE MOTOR (Graham: LA7EB)
LA7F4	equ	$b999	%BLKOUT% to tape
LA82A	equ	$be12	%CBOUT% output byte to tape
LA88D	equ	$b9df	Set LoRes Pixel
LA8B5	equ	$ba07	Reset LoRes Pixel
LA8D9	equ	$ba28	Calc LoRes Pixel Pos
LA928	equ	$ba77     Clear text screen
LA92A	equ	$ba79	Clear text screen with B
LA951	equ	$baa0     JUMP TO 'SOUND' - DO A BEEP
LA974	equ	$bac3     Audio off
LA976	equ	$bac5     Audio on
LA985	equ	$bad4	Reset DAC
LA987	equ	$bad6	Writes A to DAC
LA99D	equ	$baec	Audio on (B=0)
LA9A2	equ	$bd41     %SNDSEL%
LA9DE	equ	$bd52	%JOYIN%
LAC37	equ	$8335	SEE IF ENOUGH ROOM IN RAM
LAC44	equ	$8342	OM ERROR
LAC46	equ	$8344     System error
LAC60	equ	$835e	JUMP TO BASIC'S ERROR HANDLER
LAC73	equ	$8371     OK prompt
LAC7C	equ	$837a	GO TO BASIC'S MAIN LOOP, IT WILL LOAD PROGRAM
LACEF	equ	$83ed     BasVect2
LAD19	equ	$8417     Erases program
LAD21	equ	$841f     BasVect1
LAD33	equ	$8434     Resets stack
LAD9E	equ	$849f     RUN BASIC
LADC6	equ	$84dc	LOOP THROUGH BASIC'S MAIN INTERPRETATION LOOP
LADD4	equ	$84ed	JUMP TO BASIC'S COMMAND HANDLER
LADEB	equ	$851b	GO DO A BREAK CHECK IF A KEY IS DOWN
LADFB	equ	$852b	wait for key press, return in A
LAE15	equ	$8545	GO 'STOP' THE SYSTEM
LAF9A	equ	$86cd
LAFA4	equ	$86da	BRANCH BACK TO BASIC’S 'LET' COMMAND
LAFB1	equ	$86e7	MOVE IT INTO THE STRING SPACE
LB00C	equ	$9dc9	check for ,"
LB01E	equ	$9de3	?not sure, in INPUT
LB069	equ	$879a
LB143	equ	$8874	NUMBER TYPE CHECK
LB146	equ	$8877	'TM' ERROR IF NUMERIC VARIABLE, Get expression
LB148	equ	$8879	DO A 'TM' CHECK
LB156	equ	$8887	EVALUATE AN EXPRESSION, Get string
LB166	equ	$8897	(eqv ROM but usage?) COMING FROM THE 'LET' COMMAND
LB244	equ	$8975	STRIP PROMPT STRING FROM BASIC AND PUT IT ON THE STRING STACK
LB262	equ	$899f	SYNTAX CHECK FOR '(' AND EVALUATE EXPRESSION
LB267	equ	$89a4	CkClBrak
LB26A	equ	$89a7	CkOpBrak
LB26D	equ	$89aa	CkComa
LB26F	equ	$89ac	CkChar
LB277	equ	$89b4	SYNTAX ERROR
LB2CE	equ	$8a0b	JUMP TO BASIC'S SECONDARY COMMAND HANDLER
LB357	equ	$8a94     %GETVAR%
LB3E6	equ	$8b23	EVALUATE EXPRESSION, RETURN VALUE IN ACCD
LB3E9	equ	$8b29	%GETUSR%
LB44A	equ	$8b8d	FC ERROR
LB4F2	equ	$8c35	Assign16Bit (skips to $8c37)
LB4F3	equ	$8c36     Assign8Bit, LOAD ACCB INTO FPA0
LB50F	equ	$8c52	RESERVE ACCB BYTES IN STRING SPACE
LB516	equ	$8c59	PUT ON TEMPORARY STRING STACK
LB591	equ	$8cd7	Force string space garbage collection
LB654	equ	$8d9a	GET LENGTH AND ADDRESS OF STRING
LB657	equ	$8d9d	PURGE THE STRING PUT ON THE STRING STACK
LB659	equ	$8d9f	%DELVAR%
LB69B	equ	$8de1	SAVE STRING DESCRIPTOR ON STRING STACK
LB6A4	equ	$8dea	FIRST BYTE OF STRING EXPRESSION
LB70B	equ	$8e51	Get8Bit
LB70E	equ	$8e54	EVALUATE NUMERIC EXPRESSION AND RETURN VALUE IN ACCB
LB738	equ	$8e7e	SYNTAX CHECK FOR COMMA, EVALUATE EXPRESSION
LB73D	equ	$8e83     Get16Bit
LB764	equ	$8eaa	LIST token dispatch address
LB958	equ	$90a1     Print CR
LB95C	equ	$90a5	SEND A CR TO THE SCREEN
LB99C	equ	$90e5	STRINOUT
LB99F	equ	$90e8     STRINOUT+3, PRINT STRING TO CONSOLE OUT
LB9A2	equ	$90eb	SEND STRING TO CONSOLE OUT
LB9AC	equ	$90f5	SEND BLANK TO CONSOLE OUT
LB9AF	equ	$90f8	SEND A '?' TO THE SCREEN
LB9C5	equ	$910E	oper_plus_dispatch
LBB91	equ	$933c	oper_div_dispatch, DIVIDE FPA1 BY FPA0
LBC14	equ	$93bf	MOVFM, COPY A PACKED FP NUMBER FROM (X) TO FPA0
LBC33	equ	$93de	PACK FPA0 INTO VARDES
LBC35	equ	$93e0	PACK FPA0 AND STORE IT IN STRING SPACE
LBC5F	equ	$940a	SAVE NUMBER IN FPA1
LBDCC	equ	$957a     Print Number,  Print decimal D to screen
LBDD9	equ	$9587	CONVERT FP NUMBER TO ASCII STRING

* Locations referenced by function name in ecb_equates.asm
* name	equ	Dragon	Coco
STRINOUT	equ	$90e5	LB99C
EVALEXPB	equ	$8e51 	LB70B	Get8Bit, Evaluate argument
SYNCOMMA	equ	$89aa	LB26D	CkComa
XBWMST	equ	$b44f	L80C0
CHROUT	equ	$a002	LA002
PUTCHR	equ	$b54a	LA282
CLS	equ	$ba5f	LA910
INTCNV	equ	$8b2d	LB3ED
GIVABF	equ	$8c37	LB4F4
LIST	equ	$8eaa	LB764
INT	equ	$9499	LBCEE

* Hooks for extended basic, just point to an RTS location on Dragon
* A similar entry point is indicated in the comment
XVEC3	equ	$b64b	(rts) $b54b	Character output
XVEC4	equ	$b64b	(rts) $b50b	Character input
XVEC8	equ	$b64b	(rts) $b664	Close a single device or file
XVEC9	equ	$b64b	(rts) $84de	?About to deal with first char of new statement /PRINT
XVEC15	equ	$b64b	(rts) $8954	Evaluate an expression
XVEC17	equ	$b64b	(rts) $8347	System error
XVEC18	equ	$b64b	(rts) $85a5	RUN statement

