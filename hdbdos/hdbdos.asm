*******************************************************************
* HDB-DOS - RS-DOS Hard Disk Driver
*
* For DISK EXTENDED COLOR BASIC VERSION 1.1
* Copyright (C) 2004 Boisy G. Pitre
*
* $Id: hdbdos.asm,v 1.18 2009/03/19 23:57:42 boisy Exp $
*
* Comments                                              Who YY/MM/DD
* ------------------------------------------------------------------
* Disassembled from latest binary from Roger Krupski    RG  02/04/14
*
* Fixed IDE problems with Maxtor 7540AV drive related   BGP 02/09/24
* to timing -- note that the Glenside IDE controller
* does NOT work with an MPI using this current
* version of HDB-DOS.  It may seem to work, but there
* are data corruption issues when READing.
*
* Rearranged code a bit, made tighter and better        BGP 02/10/01
* and now DRIVE# will timeout with ?IO ERROR if the
* selected drive doesn't exist, instead of locking
* up in a forever loop.
*
* Added OR of #$80 with SCSI ID in order for HDB-DOS    BGP 02/10/05
* to work with Quantum Fireball 540S drive
*
* Default drive is now set to  DCDRV just before        BGP 02/10/07
* AUTOEXEC.BAS
*
* Fixed bug where computation for last hard drive was   BGP 02/11/04
* flawed because X wasn't being preserved when calling
* PRECMD (affected SCSI version only)
************** HDB-DOS 1.1 RELEASED HERE! *************
*
************** HDB-DOS 1.1A MODIFICATIONS *************
*                                                       BGP 04/05/18
* Entire code base optimized code extensively for size.
* IDE:  DRIVE STOP/DRIVE RESTORE are now NO-OPs
* SCSI: 10 byte SCSI packets such as READ CAPACITY now clear all 10 bytes
*
* SCSI Drive Connor CP30200 doesn't work because after the receipt of the
* first SCSI command, the drive sends a SDTR
*
************** HDB-DOS 1.1B MODIFICATIONS *************
*                                                       BGP 04/06/15
* More clever IDE optimizations to save space.
*
************** HDB-DOS 1.1C MODIFICATIONS *************
*                                                       BGP 07/11/21
* IDE Only: Made timeout code for checking BUSY a common routine called
* at HDINIT time and at EXECMD time.
*
************** HDB-DOS 1.1D MODIFICATIONS *************
*                                                       BGP 07/12/03
* IDE Only: Added code to wait for !BUSY *BEFORE* sending EXECDIAG command
* to IDE device.  The PQI 256MB CF now comes up consistently.
*
*                                                       BGP 08/03/24
* DriveWire Only: Added code to use OP_READX for anticipated DriveWire X
* version that may come out if 115.2Kbps comes to life.  To assemble
* this version, add DW as a conditional.
*
************** HDB-DOS 1.2 MODIFICATIONS *************
*                                                       BGP ??/??/??
* Don't remember...
*
************** HDB-DOS 1.3 MODIFICATIONS *************
*                                                       BGP 04/15/2011
* DriveWire Only: Added code for DW4 (230Kbps) from Darren A.
*
************** MAJOR SOURCE RENOVATIONS  *************
*                                                       BGP 01/19/2012
* All ROMs now built from a single source file
*
************** HDB-DOS 1.4 MODIFICATIONS *************
*                                                       BGP 04/18/2012
* Added DWREAD/DWWRITE vectors
*

* DSKCON Equates
DCSEC          equ       $ED                 DSKCON Sector
DCSTAT         equ       $F0                 DSKCON Status



FATS           equ       $800




SKIP2          equ       $8C

               IFDEF     DW
Vi.PkSz        equ       0
V.SCF          equ       0
               use       dwdefs.d

RBLK           equ       OP_READEX           Read Block
RRBLK          equ       OP_REREADEX         ReRead Block
WBLK           equ       OP_WRITE            Write Block


Carry          equ       1
IntMasks       equ       $50
E$CRC          equ       243
E$NotRdy       equ       246
PIA1Base       equ       $FF20
DATAADDR       equ       PIA1Base
TDELAY         equ       8                   Retries in the case of DriveWire
MAXDN          equ       255
               ENDC

               IFDEF     IDE
* IDE Command Opcode Equates
RSTR           equ       $10                 Recalibrate
STSTOP         equ       $10                 Recalibrate
RBLK           equ       $20                 Read block
WBLK           equ       $30                 Write block
SEEK           equ       $70                 Seek
RCAPY          equ       $EC                 Identify device
EXECDIAG       equ       $90                 Execute device diagnostic

* Misc Equates
MAXDN          equ       2                   Max. no. of IDE drives
TDELAY         equ       $C0                 IDE startup timeout delay
ACK            equ       0                   Necessary because used as SCSI conditional

               IFDEF     USELBA
DEVBYT         equ       %11100000
               ENDC
               IFDEF     USECHS
DEVBYT         equ       %10100000
               ENDC

* IDE status Register Equates
BUSY           equ       %10000000           Drive busy
DREADY         equ       %01000000           Drive ready
WRITFL         equ       %00100000           Wwrite fault
SEEKDN         equ       %00010000           Seek done
DATARQ         equ       %00001000           Data request
CRDATA         equ       %00000100           Corrected data was done
INDEX          equ       %00000010           Revolution complete
ERROR          equ       %00000001           Error detected

* IDE Hardware Offsets
DATAADDR       equ       $FF70
DATARW         equ       0                   8 bits (1st 8, non-latched)
ERR            equ       1                   Error when read
FEATUR         equ       1                   Write
SECTCN         equ       2                   Sector count
SECTNM         equ       3                   Sector number
CYLLO          equ       4                   Low cylinder
CYLHI          equ       5                   High cylinder
DEVHED         equ       6                   Device/Heads
STATUS         equ       7                   Status when read
CMD            equ       7                   Command when write
LATCH          equ       8                   Latch (2nd 8 bits of 16 bit word)
               ENDC

               IFDEF     SCSI
* SCSI command Opcode Equates
RSTR           equ       1                   Re-zero unit
RDET           equ       3                   Request Sense
RBLK           equ       8                   Read
WBLK           equ       $0A                 Write
SEEK           equ       $0B                 Seek
STSTOP         equ       $1B                 Start/Stop (Park)
RCAPY          equ       $25                 Read Capacity

* Completion Status Masks
ERROR          equ       2                   1 = Check Sense (Error)
BUSY           equ       8                   1 = Unit Busy

* Misc Equates
MAXDN          equ       8                   Maximum number of SCSI drives (0-7)
TDELAY         equ       $0A                 SCSI bus timeout delay
               ENDC

               IFDEF     TC3                 Cloud-9 TC^3 SCSI
* TC^3 SCSI Status Register Equates
REQ            equ       $01                 1 = Request asserted
BSY            equ       $02                 1 = Controller busy
MSG            equ       $04                 1 = Message phase
CMD            equ       $08                 1 = Command, 0 = Data
INOUT          equ       $10
SEL            equ       $00                 Unused
ACK            equ       $00                 Unused

* TC^3 SCSI Host Adapter Register Equates
DATAADDR       equ       $FF70               Bi-directional data port
DATARW         equ       0                   Data read/write
STATUS         equ       1                   Read-only SCSI status
SELECT         equ       1                   Set (-SEL)ECT D-Flop
               ENDC

               IFDEF     LRTECH              LR-Tech
* LR-Tech SCSI Status Register Equates
REQ            equ       $01                 1 = Request asserted
BSY            equ       $02                 1 = Controller busy
MSG            equ       $04                 1 = Message phase
CMD            equ       $08                 1 = Command, 0 = Data
INOUT          equ       $10                 1 = Input, 0 = Output
ACK            equ       $00                 Not used
SEL            equ       $00                 Not used
RST            equ       $00                 Not used


* LR-Tech SCSI Host Adapter Register Equates
DATAADDR       equ       $FF74               Bi-directional data port
DATARW         equ       0                   Data read/write
STATUS         equ       1                   Read-only SCSI status
SELECT         equ       2                   Set (-SEL)ECT D-Flop
SCSIRESET      equ       3                   Send SCSI -RST
               ENDC

               IFDEF     KENTON              Ken-Ton SCSI
* Ken-Ton SCSI Status Register Equates
REQ            equ       $01                 1 = Request asserted
BSY            equ       $02                 1 = Controller busy
MSG            equ       $04                 1 = Message phase
CMD            equ       $08                 1 = Command, 0 = Data
INOUT          equ       $10                 1 = Input, 0 = Output
ACK            equ       $20                 1 = Ack asserted
SEL            equ       $40                 1 = Select asserted
RST            equ       $80                 1 = Reset asserted


* Ken-Ton SCSI Host Adapter Register Equates
DATAADDR       equ       $FF74               Bi-directional data port
DATARW         equ       0                   Data read/write
STATUS         equ       1                   Read-only SCSI status
SELECT         equ       2                   Set (-SEL)ECT D-Flop
SCSIRESET      equ       3                   Send SCSI -RST
               ENDC


               IFDEF     DHDII               Disto Hard Disk II
* Disto SASI status Register Equates
REQ            equ       $80                 1 = Request asserted
BSY            equ       $01                 1 = Controller busy
MSG            equ       $00                 Unused
CMD            equ       $40                 1 = Command, 0 = Data
INOUT          equ       $20                 1 = Input, 0 = Output
ACK            equ       $02                 1 = Ack asserted
SEL            equ       $00                 Unused
RST            equ       $00                 Unused


* Disto SASI Host Adapter Register Equates
DATAADDR       equ       $FF53               Bi-directional data port
DATARW         equ       0                   Data read/write
STATUS         equ       -2                  Read SASI status
SELECT         equ       -1                  Write -SEL pulse
SCSIRESET      equ       -2                  Write -RST pulse
               ENDC

               IFDEF     D4N1                Disto 4-N-1 SCSI
* Disto 4-N-1 SCSI Status Register Equates
REQ            equ       $80                 1 = Request asserted
BSY            equ       $01                 1 = Controller busy
MSG            equ       $00                 Unused
CMD            equ       $40                 1 = Command, 0 = Data
INOUT          equ       $20                 1 = Input, 0 = Output
ACK            equ       $02                 1 = Ack asserted
SEL            equ       $00                 Unused
RST            equ       $00                 Unused


* Disto 4-N-1 SCSI Host Adapter Register Equates
DATAADDR       equ       $FF5B               Bi-directional data port
DATARW         equ       0                   Data read/write
STATUS         equ       -2                  Read SASI status
SELECT         equ       -1                  Write -SEL pulse
SCSIRESET      equ       -2                  Write -RST pulse
               ENDC

               IFDEF     DRAGON
* Dragon Color Basic tokens
TOKEN_ON       equ       $88
TOKEN_RESTORE  equ       $90
TOKEN_STOP     equ       $92
TOKEN_CLOAD    equ       $99
TOKEN_CSAVE    equ       $9A
TOKEN_POS      equ       $83
TOKEN_TO       equ       $BC
TOKEN_OFF      equ       $C2
TOKEN_EQUAL    equ       $CB
TOKEN_PMODE    equ       $B7
TOKEN_DLOAD    equ       $B9
               ELSE
* Coco Extended Color Basic tokens
TOKEN_ON       equ       $88
TOKEN_RESTORE  equ       $8F
TOKEN_STOP     equ       $91
TOKEN_CLOAD    equ       $97
TOKEN_CSAVE    equ       $98
TOKEN_POS      equ       $9A
TOKEN_TO       equ       $A5
TOKEN_OFF      equ       $AA
TOKEN_EQUAL    equ       $B3
TOKEN_PMODE    equ       $C8
TOKEN_DLOAD    equ       $CA
               ENDC

* Disk Color BASIC tokens
TOKEN_AS       equ       $A7
TOKEN_DRIVE    equ       $CF

* Disk Color BASIC 1.1
* Copied from the PDF version of Disk Color BASIC Unravelled.
* Fixed up to assemble in Mamou
*
*  Revision History
*
*# $Id: $
*
DHITOK         equ       $E1                 HIGHEST 1.1 DISK TOKEN
DHISTOK        equ       $A7                 HIGHEST 1.1 DISK SECONDARY TOKEN
CYEAR          equ       '2
*
*
*
**
**** FILE ALLOCATION TABLE FORMAT
**
*
* THE FILE ALLOCATION TABLE (FAT) CONTAINS THE STATUS OF THE GRANULES ON A DISKETTE.
* THE FAT CONTAINS 6 CONTROL BYTES FOLLOWED BY 68 DATA BYTES (ONE PER GRANULE). ONLY THE
* FIRST TWO OF THE SIX CONTROL BYTES ARE USED. A VALUE OF $FF IS SAVED IN UNALLOCATED
* GRANULES. IF BITS 6 & 7 OF THE DATA BYTE ARE SET, THE GRANULE IS THE LAST GRANULE
* IN A FILE AND BITS 0-5 ARE THE NUMBER OF USED SECTORS IN THAT GRANULE. IF BITS 6 & 7
* ARE NOT SET, THE DATA BYTE CONTAINS THE NUMBER OF THE NEXT GRANULE IN THE FILE.
* OFFSETS TO FAT CONTROL BYTES
FAT0           equ       0                   ACTIVE FILE COUNTER : DISK TO RAM FAT IMAGE DISABLE
FAT1           equ       1                   VALID DATA FLAG: 0=DISK DATA VALID, <> 0 = NEW FAT
*			DATA - DISK DATA INVALID
*	2 TO 5		NOT USED
FATCON         equ       6                   OFFSET TO START OF FAT DATA (68 BYTES)
*
**
**** DIRECTORY ENTRY FORMAT
**
*
* THE DIRECTORY IS USED TO KEEP TRACK OF HOW MANY FILES ARE STORED ON A DISKETTE
* AND WHERE THE FILE IS STORED ON THE DISK. THE FIRST GRANULE USED BY THE FILE WILL
* ALLOW THE FAT TO TRACK DOWN ALL OF THE GRANULES USED BY THE FILE. IF THE FIRST
* BYTE OF THE DIRECTORY ENTRY IS ZERO, THE FILE HAS BEEN KILLED;
* IF THE FIRST BYTE IS $FF THEN THE DIRECTORY ENTRY HAS NEVER BEEN USED.
*
* BYTE DESCRIPTION
DIRNAM         equ       0                   FILE NAME
DIREXT         equ       8                   FILE EXTENSION
DIRTYP         equ       11                  FILE TYPE
DIRASC         equ       12                  ASCII FLAG
DIRGRN         equ       13                  FIRST GRANULE IN FILE
DIRLST         equ       14                  NUMBER OF BYTES IN LAST SECTOR
*	16 TO 31		UNUSED
*
**
**** FILE CONTROL BLOCK FORMAT
**
*
* THE FILE STRUCTURE OF COLOR TRS DOS IS CONTROLLED BY A FILE CONTROL BLOCK (FCB)
* THE FCB CONTAINS 25 CONTROL BYTES AND A SECTOR LONG (256 BYTES) DATA BUFFER.
* THE CONTROL BYTES CONTROL THE ORDERLY FLOW OF DATA FROM THE COMPUTER'S RAM TO
* THE DISKETTE AND VICE VERSA. THE OPEN COMMAND INITIALIZES THE FCB; THE INPUT,
* OUTPUT, WRITE, PRINT, GET AND PUT COMMANDS TRANSFER DATA THROUGH THE FCB AND
* THE CLOSE COMMAND TURNS OFF THE FCB.
* TABLES OF OFFSETS TO FCB CONTROL BYTES
***** RANDOM FILE
*		BYTE	DESCRIPTION
FCBTYP         equ       0                   FILE TYPE: $40=RANDOM/DIRECT, 0=CLOSED
FCBDRV         equ       1                   DRIVE NUMBER
FCBFGR         equ       2                   FIRST GRANULE IN FILE
FCBCGR         equ       3                   CURRENT GRANULE BEING USED
FCBSEC         equ       4                   CURRENT SECTOR BEING USED (1-9)
* 		5	UNUSED
FCBPOS         equ       6                   CURRENT PRINT POSITION - ALWAYS ZERO IN RANDOM FILES
FCBREC         equ       7                   CURRENT RECORD NUMBER
FCBRLN         equ       9                   RANDOM FILE RECORD LENGTH
FCBBUF         equ       11                  POINTER TO START OF THIS FILE'S RANDOM ACCESS BUFFER
FCBSOF         equ       13                  SECTOR OFFSET TO CURRENT POSITION IN RECORD
FCBFLG         equ       15                  GET/PUT FLAG: 0=PUT, 1=PUT
* 		16,17	NOT USED
FCBDIR         equ       18                  DIRECTORY ENTRY NUMBER (0-71)
FCBLST         equ       19                  NUMBER OF BYTES IN LAST SECTOR OF FILE
FCBGET         equ       21                  'GET' RECORD COUNTER: HOW MANY CHARACTERS HAVE BEEN PULLED OUT OF THE CURRENT RECORD
FCBPUT         equ       23                  'PUT' RECORD COUNTER: POINTER TO WHERE IN THE RECORD THE NEXT BYTE WILL BE 'PUT'
*FCBCON	EQU	25	OFFSET TO START OF FCB DATA BUFFER (256 BYTES)
***** SEQUENTIAL FILE
* 		BYTE	DESCRIPTION
*FCBTYP	EQU	0	FILE TYPE: $10=INPUT, $20=OUTPUT, 0=CLOSED
*FCBDRV	EQU	1	DRIVE NUMBER
*FCBFGR	EQU	2	FIRST GRANULE IN FILE
*FCBCGR	EQU	3	CURRENT GRANULE BEING USED
*FCBSEC	EQU	4	CURRENT SECTOR BEING USED (1-9)
FCBCPT         equ       5                   INPUT FILE: CHARACTER POINTER - POINTS TO NEXT CHARACTER IN
* FILE TO BE PROCESSED.
* OUTPUT FILE: FULL SECTOR FLAG - IF IT IS 1 WHEN THE FILE IS
* CLOSED IT MEANS 256 BYTES OF THE LAST SECTOR HAVE BEEN USED.
*FCBPOS	EQU	6	CURRENT PRINT POSITION
*FCBREC	EQU	7	CURRENT RECORD NUMBER: HOW MANY WHOLE SECTORS HAVE BEEN
* INPUT OR OUTPUT TO A FILE.
* 9 TO 15 UNUSED
FCBCFL         equ       16                  CACHE FLAG: 00=CACHE EMPTY, $FF=CACHE FULL
FCBCDT         equ       17                  CACHE DATA BYTE
*FCBDIR	EQU	18	DIRECTORY ENTRY NUMBER (0-71)
*FCBLST	EQU	19	NUMBER OF BYTES IN LAST SECTOR OF FILE
* 		21,22	UNUSED
FCBDFL         equ       23                  INPUT FILE ONLY: DATA LEFT FLAG: 0=DATA LEFT, $FF=NO DATA (EMPTY)
FCBLFT         equ       24                  NUMBER OF CHARACTERS LEFT IN BUFFER (INPUT FILE)
* NUMBER OF CHARS STORED IN BUFFER (OUTPUT FILE)
FCBCON         equ       25                  OFFSET TO FCB DATA BUFFER (256 BYTES)
               IFDEF     ORG
               org       ORG
MAGICDG        fcc       'OS'
               ELSE
               org       $C000
MAGICDG        fcc       'DK'
               ENDC
LC002          bra       LC00C
DCNVEC         fdb       DSKCON              DSKCON POINTER
DSKVAR         fdb       DCOPC               ADDRESS OF DSKCON VARIABLES
DSINIT         fdb       DOSINI              DISK INITIALIZATION VECTOR
DOSVEC         fdb       DOSCOM              DOS COMMAND VECTOR
**** ZERO OUT THE RAM USED BY DISK BASIC
LC00C          ldx       #DBUF0              POINT X TO START OF DISK RAM
LC00F          clr       ,X+                 CLEAR A BYTE
               cmpx      #DFLBUF             END OF DISK'S RAM?
               bne       LC00F               NO - KEEP CLEARING
               ldx       #LC109              POINT X TO ROM IMAGE OF COMMAND INTERPRETATION TABLE
               IFDEF     DRAGON
               ldu       #COMVEC+10          POINT U TO RAM ADDRESS OF SAME (STUB1 ON DRAGON)
               ELSE
               ldu       #COMVEC+20          POINT U TO RAM ADDRESS OF SAME (STUB2 ON COCO)
               ENDC
               ldb       #10                 10 BYTES PER TABLE
               jsr       >LA59A              MOVE (B) BYTES FROM (X) TO (U)
               ldd       #LB277              SYNTAX ERROR ADDRESS
               std       $03,U               * SET JUMP TABLE ADDRESSES OF THE USER COMMAND
               std       $08,U               * INTERPRETATION TABLE TO POINT TO SYNTAX ERROR
               clr       ,U                  CLEAR BYTE 0 OF USER TABLE (DOESN'T EXIST FLAG)
               clr       $05,U               SET NUMBER OF SECONDARY USER TOKENS TO ZERO
               IFNDEF    DRAGON
               ldd       #DXCVEC             * SAVE NEW
               std       COMVEC+13           * POINTERS TO EXBAS
               ldd       #DXIVEC             * COMMAND AND SECONDARY
               std       COMVEC+18           * COMMAND INTERPRETATION ROUTINES
               ELSE
               FILL      $12,12              FILL WITH NOP (TO HAVE SAME CODE LENGTH AS COCO BUILD)
               ENDC
**** MOVE THE NEW RAM VECTORS FROM ROM TO RAM
               ldu       #RVEC0              POINT U TO 1ST RAM VECTOR
LC03B          lda       #$7E                OP CODE OF JMP INSTRUCTION
               sta       RVEC22              SET 1ST BYTE OF 'GET'/'PUT' RAM VECTOR TO 'JMP'
               sta       ,U+                 SET 1ST BYTE OF RAM VECTOR TO 'JMP'
               ldd       ,X++                GET RAM VECTOR FROM ROM
               std       ,U++                STORE IT IN RAM
               cmpx      #LC139              COMPARE TO END OF ROM VALUES
               bne       LC03B               BRANCH IF NOT ALL VECTORS MOVED
               ldx       #DVEC22             GET ROM VALUE OF 'GET'/'PUT' RAM VECTOR
               stx       RVEC22+1            SAVE IT IN RAM
               ldx       #DVEC20             GET DISK COMMAND INTERPRETATION LOOP RAM VECTOR
               stx       RVEC20+1            SAVE IN RAM VECTOR TABLE
**** INITIALIZE DISK BASIC'S USR VECTORS
               ldx       #DUSRVC             POINT X TO START OF DISK BASIC USR VECTORS
               stx       USRADR              SAVE START ADDRESS IN USRADR
               ldu       #LB44A              POINT U TO ADDRESS OF 'FUNCTION CALL' ERROR
               ldb       #$0A                10 USER VECTORS TO INITIALIZE
LC061          stu       ,X++                SET USR VECTOR TO 'FC' ERROR
               decb                          DECREMENT USR VECTOR COUNTER
               bne       LC061               BRANCH IN NOT DONE WITH ALL 10 VECTORS
               ldx       #DNMISV             GET ADDRESS OF NMI SERVICING ROUTINE
               stx       NMIVEC+1            SAVE IT IN NMI VECTOR
               lda       #$7E                OP CODE OF JMP
               sta       NMIVEC              MAKE THE NMI VECTOR A JMP
               ldx       #DIRQSV             GET ADDRESS OF DISK BASIC IRQ SERVICING ROUTINE
               stx       IRQVEC+1            SAVE IT IN IRQVEC
               lda       #$13                = INITIALIZE WRITE FAT
               sta       WFATVL              = TO DISK TRIGGER VALUE
               clr       FATBL0              *
               clr       FATBL1              * INITIALIZE THE ACTIVE FILE COUNTER OF
               clr       FATBL2              * EACH FAT TO ZERO. THIS WILL CAUSE THE FATS
               clr       FATBL3              * TO THINK THERE ARE NO ACTIVE FILES
               ldx       #DFLBUF             = GET THE STARTING ADDRESS OF THE
               stx       RNBFAD              = RANDOM FILE BUFFER FREE AREA AND DAVE IT AS THE
* = START ADDRESS OF FREE RAM FOR RANDOM FILE BUFFERS
               leax      $0100,X             SAVE 256 BYTES FOR RANDOM FILE BUFFERS INITIALLY
               stx       FCBADR              SAVE START ADDRESS OF FCBS
               leax      $01,X               * ADD ONE AND SAVE THE STARTING
               stx       FCBV1               * ADDRESS OF FCB1
               clr       FCBTYP,X            CLEAR THE FIRST BYTE OF FCB 1 (CLOSE FCB)
               leax      FCBLEN,X            POINT X TO FCB 2
               stx       FCBV1+2             SAVE ITS STARTING ADDRESS IN FCB VECTOR TABLE
               clr       FCBTYP,X            CLEAR THE FIRST BYTE OF FCB 2 (CLOSE FCB)
               leax      FCBLEN,X            * POINT X TO SYSTEM FCB - THIS FCB WILL ONLY
* * BE USED TO COPY, LOAD, SAVE, MERGE, ETC
               stx       FCBV1+4             SAVE ITS ADDRESS IN THE FCB VECTOR TABLE
               clr       FCBTYP,X            CLEAR THE FIRST BYTE OF SYSTEM FCB (CLOSE FCB)
               lda       #$02                * SET THE NUMBER OF ACTIVE RESERVED
               sta       FCBACT              * FILE BUFFERS TO 2 (1,2)
               leax      FCBLEN,X            POINT X TO ONE PAST THE END OF SYSTEM FCB
               tfr       X,D                 SAVE THE ADDRESS IN ACCD
               tstb                          ON AN EVEN 256 BYTE BOUNDARY?
               beq       LC0BD               YES
               inca                          NO - ADD 256 TO ADDRESS
LC0BD          bita      #$01                * CHECK TO SEE IF ACCD IS ON AN EVEN
               beq       LC0C2               * 512 BYTE (ONE GRAPHIC PAGE) BOUNDARY - ADD
               inca                          * 256 (INCA) TO IT IF NOT
LC0C2          tfr       A,B                 COPY ACCA TO ACCB
               addb      #$18                SAVE ENOUGH ROOM FOR 4 GRAPHICS PAGES (PCLEAR 4)
               stb       TXTTAB              SAVE NEW START OF BASIC ADDRESS
               jsr       >L96EC              INITIALIZE EXBAS VARIABLES & DO A NEW
               lda       BEGGRP              GET THE START OF CURRENT GRAPHICS PAGE
               adda      #$06                ADD 1.5K (6 X 256 = ONE GRAPHICS PAGE)
               sta       ENDGRP              SAVE NEW END OF GRAPHICS PAGE
               jsr       [DSINIT]            INITIALIZE SWI2,3 JUMP ADDRESSES
               bsr       LC0F0               GO INITIALIZE THE FLOPPY DISK CONTROLLER
               andcc     #$AF                TURN ON IRQ AND FIRQ
*	LDX	#LC139-1	POINT X TO DISK BASIC COPYRIGHT MESSAGE
               ldx       #LC139-1            COPYRIGHT MESSAGE - 1
LC0DC          jsr       STRINOUT            PRINT COPYRIGHT MESSAGE TO SCREEN
               ldx       #DKWMST             GET DISK BASIC WARM START ADDRESS
               stx       RSTVEC              SAVE IT IN RESET VECTOR
*	JMP	>LA0E2	JUMP BACK TO BASIC
               jmp       >HDINIT
DKWMST         nop                           WARM START INDICATOR
               bsr       LC0F0               INITIALIZE THE FLOPPY DISK CONTROLLER
               jsr       >LD2D2              CLOSE FILES AND DO MORE INITIALIZATION
               jmp       XBWMST              JUMP TO EXBAS' WARM START
LC0F0          clr       NMIFLG              RESET NMI FLAG
               clr       RDYTMR              RESET DRIVE NOT READY TIMER
               clr       DRGRAM              RESET RAM IMAGE OF DSKREG (MOTORS OFF)
               clr       DSKREG              RESET DISK CONTROL REGISTER
               lda       #$D0                FORCE INTERRUPT COMMAND OF 1793
               sta       FDCREG              SEND IT TO 1793
LC101          nop       
               lbsr      LCD01
*	EXG	A,A	* DELAY
*	EXG	A,A	* DELAY SOME MORE
               lda       FDCREG              GET 1793 STATUS (CLEAR REGISTER)
               rts       
* DISK BASIC COMMAND INTERP TABLES
LC109          fcb       20                  20 DISK BASIC 1.1 COMMANDS
               fdb       LC192               DISK BASIC'S COMMAND DICTIONARY
               fdb       LC238               COMMAND JUMP TABLE
               fcb       06                  6 DISK BASIC SECONDARY FUNCTIONS
               fdb       LC219               SECONDARY FUNCTION TABLE
               fdb       LC24E               SECONDARY FUNCTION JUMP TABLE
* RAM HOOKS FOR DISK BASIC
LC113          fdb       DVEC0,DVEC1,DVEC2
*	FDB	DVEC3,DVEC4,DVEC5
               fdb       DVEC3,FLEXKY,DVEC5
               fdb       DVEC6,DVEC7,DVEC8
               fdb       XVEC9,DVEC10,DVEC11
               fdb       DVEC12,DVEC13,DVEC14
               fdb       DVEC15,DVEC12
*	FDB	DVEC17,DVEC18
               fdb       DVEC17
               fdb       RUNM
* DISK BASIC COPYRIGHT MESSAGE
LC139          fcc       'DISK EXTENDED COLOR BASIC 1.1'
               fcb       CR
               fcc       'COPYRIGHT (C) 198'
               fcb       CYEAR
               fcc       ' BY TANDY'
               fcb       CR
               fcc       'UNDER LICENSE FROM MICROSOFT'
               fcb       CR,CR,0
* DISK BASIC COMMAND DICTIONARY TABLE
* TOKEN #
LC192          fcs       'DIR'               CE
               fcs       'DRIVE'             CF
               fcs       'FIELD'             D0
               fcs       'FILES'             D1
               fcs       'KILL'              D2
               fcs       'LOAD'              D3
               fcs       'LSET'              D4
               fcs       'MERGE'             D5
               fcs       'RENAME'            D6
               fcs       'RSET'              D7
               fcs       'SAVE'              D8
               fcs       'WRITE'             D9
               fcs       'VERIFY'            DA
               fcs       'UNLOAD'            DB
               fcs       'DSKINI'            DC
               fcs       'BACKUP'            DD
               fcs       'COPY'              DE
               fcs       'DSKI$'             DF
               fcs       'DSKO$'             E0
               fcs       'DOS'               E1
* DISK BASIC COMMAND JUMP TABLE
* COMMAND / TOKEN #
LC1F1          fdb       DIR                 DIR / CE
               fdb       DRIVE               DRIVE / CF
               fdb       FIELD               FIELD / D0
               fdb       FILES               FILES / D1
               fdb       KILL                KILL / D2
               fdb       LOAD                LOAD / D3
               fdb       LSET                LSET / D4
               fdb       MERGE               MERGE / D5
               fdb       RENAME              RENAME / D6
               fdb       RSET                RSET / D7
               fdb       SAVE                SAVE / D8
               fdb       WRITE               WRITE / D9
               fdb       VERIFY              VERIFY / DA
               fdb       UNLOAD              UNLOAD / DB
               fdb       DSKINI              DSKINI /DC
               fdb       BACKUP              BACKUP / DD
               fdb       COPY                COPY / DE
               fdb       DSKI                DSKI$ / DF
               fdb       DSKO                DSKO$ / E0
               fdb       DOS                 DOS / E1
* SECONDARY FUNCTION DICTIONARY TABLE
* TOKEN #
LC219          fcs       'CVN'               A2
               fcs       'FREE'              A3
               fcs       'LOC'               A4
               fcs       'LOF'               A5
               fcs       'MKN$'              A6
               fcs       'AS'                A7
* DISK BASIC SECONDARY FUNCTION JUMP TABLE
* FUNCTION / TOKEN #
LC22C          fdb       CVN                 CVN / A2
               fdb       FREE                FREE / A3
               fdb       LOC                 LOC / A4
               fdb       LOF                 LOF / A5
               fdb       MKN                 MKN$ / A6
               fdb       LB277               AS / A7
*DISK BASIC COMMAND INTERPRETATION HANDLER
LC238          cmpa      #DHITOK             *COMPARE TO HIGHEST DISK BASIC TOKEN
               bhi       LC244               *AND BRANCH IF HIGHER
               ldx       #LC1F1              POINT X TO DISK BASIC COMMAND JUMP TABLE
               suba      #$CE                SUBTRACT OUT LOWEST DISK BASIC COMMAND TOKEN
               jmp       >LADD4              JUMP TO BASIC'S COMMAND HANDLER
LC244          cmpa      #DHITOK             COMPARE TO HIGHEST DISK BASIC TOKEN
               lbls      LB277               'SYNTAX' ERROR IF < DISK BASIC COMMAND TOKEN
               jmp       [COMVEC+33]         PROCESS A USER COMMAND TOKEN
*DISK BASIC SECONDARY COMMAND INTERPRETATION HANDLER
LC24E          cmpb      #(DHISTOK-$80)*2    *COMPARE MODIFIED SECONDARY TOKEN TO
               bls       LC256               *HIGHEST DISK BASIC TOKEN & BRANCH IF HIGHER
               jmp       [COMVEC+38]         JUMP TO USER SECONDARY COMMAND HANDLER
LC256          subb      #($A2-$80)*2        *SUBTRACT OUT THE SMALLEST SECONDARY
               pshs      B                   *DISK TOKEN & SAVE MODIFIED TOKEN ON THE STACK
               jsr       >LB262              SYNTAX CHECK FOR '(' AND EVALUATE EXPRESSION
               puls      B                   RESTORE MODIFIED TOKEN
               ldx       #LC22C              POINT X TO SECONDARY COMMAND JUMP TABLE
               jmp       >LB2CE              JUMP TO BASIC'S SECONDARY COMMAND HANDLER
* ERROR DRIVER RAM VECTOR
DVEC17         puls      Y                   PUT THE RETURN ADDRESS INTO Y
               jsr       >LAD33              RESET THE CONT FLAG, ETC
               jsr       >LD2D2              INITIALIZE SOME DISK VARIABLES AND CLOSE FILES
               pshs      Y,B                 PUT RETURN ADDRESS AND ERROR NUMBER ON THE STACK
               jsr       >DVEC7              CLOSE ALL FILES
               puls      B                   GET THE ERROR NUMBER BACK
               cmpb      #2*27               COMPARE TO THE LOWEST DISK ERROR NUMBER
               lbcs      XVEC17              BRANCH TO EXBAS ERROR HANDLER IF NOT DISK ERROR NUMBER
               leas      $02,S               PURGE RETURN ADDRESS OFF THE STACK
               jsr       >LA7E9              TURN OFF THE CASSETTE MOTOR
               jsr       >LA974              DISABLE THE ANALOG MULTIPLEXER
               clr       DEVNUM              SET DEVICE NUMBER TO THE SCREEN
               jsr       >LB95C              SEND A CR TO THE SCREEN
               jsr       >LB9AF              SEND A '?' TO THE SCREEN
               ldx       #LC290-2*27         POINT X TO DISK BASIC'S ERROR TABLE
               jmp       >LAC60              JUMP TO BASIC'S ERROR HANDLER
* DISK BASIC ERROR MESSAGES
LC290          fcc       'BR'                27 BAD RECORD
               fcc       'DF'                28 DISK FULL
               fcc       'OB'                29 OUT OF BUFFER SPACE
               fcc       'WP'                30 WRITE PROTECTED
               fcc       'FN'                31 BAD FILE NAME
               fcc       'FS'                32 BAD FILE STRUCTURE
               fcc       'AE'                33 FILE ALREADY EXISTS
               fcc       'FO'                34 FIELD OVERFLOW
               fcc       'SE'                35 SET TO NON-FIELDED STRING
               fcc       'VF'                36 VERIFICATION ERROR
               fcc       'ER'                37 WRITE OR INPUT PAST END OF RECORD
* DISK FILE EXTENSIONS
BASEXT         fcc       'BAS'               BASIC FILE EXTENSION
DEFEXT         fcc       "   "               BLANK (DEFAULT) FILE EXTENSION
DATEXT         fcc       'DAT'               DATA FILE EXTENSION
BINEXT         fcc       'BIN'               BINARY FILE EXTENSION
* CLS RAM VECTOR
DVEC22         pshs      X,CC                SAVE X REG AND STATUS
               ldx       $03,S               LOAD X WITH CALLING ADDRESS
               cmpx      #L975F              COMING FROM EXBAS' GET/PUT?
               bne       LC2BF               NO
               cmpa      #'#                 NUMBER SIGN (GET#, PUT#)?
               beq       LC2C1               BRANCH IF GET OR PUT TO RANDOM FILE
LC2BF          puls      CC,X,PC             RESTORE X REG, STATUS AND RETURN
* GET/PUT TO A DIRECT/RANDOM FILE
LC2C1          leas      $05,S               PURGE RETURN ADDRESS AND REGISTERS OFF OF THE STACK
               jsr       >LC82E              EVALUATE DEVICE NUMBER & SET FCB POINTER
               stx       FCBTMP              SAVE FCB POINTER
               clr       FCBGET,X            * RESET THE GET
               clr       FCBGET+1,X          * DATA POINTER
               clr       FCBPUT,X            = RESET THE PUT
               clr       FCBPUT+1,X          = DATA POINTER
               clr       FCBPOS,X            RESET PRINT POSITION COUNTER
               lda       FCBDRV,X            *GET THE FCB DRIVE NUMBER AND
               sta       DCDRV               *SAVE IT IN DSKCON VARIABLE
               jsr       GETCCH              GET CURRENT INPUT CHARACTER FROM BASIC
               beq       LC2EA               BRANCH IF END OF LINE
               jsr       SYNCOMMA            SYNTAX CHECK FOR COMMA
               jsr       >LB73D              EVALUATE EXPRESSION - RETURN IN (X)
               tfr       X,D                 SAVE RECORD NUMBER IN ACCD
LC2E6          ldx       FCBTMP              POINT X TO FCB
               std       FCBREC,X            SAVE RECORD NUMBER IN FCB
LC2EA          ldd       FCBREC,X            GET RECORD NUMBER
               beq       LC30B               'BAD RECORD' ERROR IF RECORD NUMBER = 0
               jsr       >LC685              INCREMENT RECORD NUMBER
               ldd       FCBRLN,X            * GET RANDOM FILE RECORD LENGTH AND RANDOM FILE
               ldx       FCBBUF,X            * BUFFER POINTER AND SAVE THEM ON THE STACK -
               pshs      X,B,A               * THESE ARE THE INITIAL VALUES OF A TEMPORARY
* * RECORD LENGTH COUNTER AND RANDOM BUFFER
* * POINTER WHICH ARE MAINTAINED ON THE STACK
               leax      $-2,U               POINT X TO (RECORD NUMBER -1)
               jsr       >L9FB5              MULT (UNSIGNED) RECORD LENGTH X (RECORD NUMBER -1)
               pshs      U,Y                 SAVE PRODUCT ON THE STACK
               lda       ,S+                 CHECK MS BYTE OF PRODUCT
               bne       LC30B               'BR' ERROR IF NOT ZERO (RECORD NUMBER TOO BIG)
               puls      X                   * PULL THE BOTTOM 3 PRODUCT BYTES OFF THE STACK;
               puls      B                   * TOP TWO IN X, BOTTOM IN ACCB; ACCB POINTS TO
* * THE FIRST BYTE OF THE SECTOR USED BY THIS RECORD,
* * (X) CONTAINS THE SECTOR OFFSET (IN WHICH SECTOR
* * FROM THE START THE BYTE IS LOCATED)
LC306          cmpx      #(35-1)*18          612 SECTORS MAX IN A RANDOM FILE
               blo       LC310               BRANCH IF RECORD LENGTH O.K.
LC30B          ldb       #2*27               'BAD RECORD' ERROR
               jmp       >LAC46              JUMP TO ERROR HANDLER
LC310          ldu       FCBTMP              POINT U TO FCB
               cmpx      FCBSOF,U            * COMPARE SAVED SECTOR OFFSET TO THE CURRENT SECTOR OFFSET
               lbeq      LC3CF               * BEING PROCESSED - DO NOT PROCESS A NEW SECTOR IF THEY ARE EQUAL
               pshs      X,B                 SAVE BYTE AND SECTOR OFFSET TO RECORD START ON STACK
               lda       FCBFLG,U            * CHECK FCB GET/PUT FLAG AND
               beq       LC324               * BRANCH IF IT WAS A GET
               clr       FCBFLG,U            FORCE GET/PUT TO 'PUT'
               ldb       #$03                DSKCON WRITE OP CODE
               bsr       LC357               GO WRITE A SECTOR - SAVE 'PUT' DATA ON DISK
* CONVERT THE SECTOR OFFSET TO A GRANULE AND SECTOR NUMBER
LC324          ldd       $01,S               * GET THE NUMBER OF SECTORS TO THE START OF
               jsr       >LC784              * THIS RECORD NUMBER AND CONVERT THEM TO A GRANULE OFFSET
               pshs      B                   SAVE GRANULE OFFSET ON THE STACK
               jsr       >LC779              MULTIPLY GRANULE NUMBER X 9 - CONVERT TO NUMBER OF SECTORS
               negb                          * NEGATE LS BYTE OF GRANULE OFFSET AND ADD THE
               addb      $03,S               * LS BYTE OF SECTOR OFFSET - ACCB = SECTOR
* * NUMBER (0-8) CORRESPONDING TO THE SECTOR NUMBER WITHIN A
* * GRANULE OF THE LAST SECTOR OF THE SECTOR OFFSET
               incb                          = ADD ONE - SECTORS SAVED IN THE FCB; START
               stb       FCBSEC,U            = AT 1 NOT 0 - SAVE IT IN THE FCB
               ldb       FCBFGR,U            GET FIRST GRANULE IN FILE
               jsr       >LC755              POINT X TO FAT
               leau      FATCON,X            POINT U TO FAT DATA
               lda       ,S                  GET NUMBER OF GRANULES OFFSET TO RECORD
               inca                          ADD ONE (COMPENSATE FOR DECA BELOW)
LC33E          leax      ,U                  POINT X TO FAT DATA
               abx                           POINT X TO CORRECT GRANULE
               deca                          DECREMENT GRANULE COUNTER
               beq       LC37B               BRANCH IF CORRECT GRANULE FOUND
               stb       ,S                  SAVE GRANULE ADDRESS ON STACK
               ldb       ,X                  GET NEXT GRANULE IN FILE
               cmpb      #$C0                LAST GRANULE IN FILE?
               blo       LC33E               NO - KEEP LOOKING
* THE GRANULE BEING SEARCHED FOR IS NOT PRESENTLY DEFINED IN THIS RANDOM FILE
               ldb       ,S                  GET OFFSET TO LAST GRANULE
               tst       VD8                 * CHECK GET/PUT FLAG
               bne       LC366               * AND BRANCH IF PUT
LC352          ldb       #2*23               'INPUT PAST END OF FILE' ERROR
               jmp       >LAC46              JUMP TO ERROR HANDLER
LC357          leax      FCBCON,U            POINT X TO FCB DATA BUFFER
* READ/WRITE A SECTOR. ENTER WITH OP CODE IN ACCB, BUFFER PTR IN X
LC35A          stb       DCOPC               SAVE DSKCON OPERATION CODE VARIABLE
               stx       DCBPT               SAVE DSKCON LOAD BUFFER VARIABLE
               leax      ,U                  POINT X TO FCB
               jsr       >LC763              CONVERT FCB TRACK AND SECTOR TO DSKCON VARIABLES
               jmp       >LD6F2              READ/WRITE A TRACK OR SECTOR
* 'PUT' DATA INTO A GRANULE NOT PRESENTLY INCLUDED IN THIS FILE
LC366          pshs      X,A                 SAVE GRANULE COUNTER AND POINTER TO LAST USED GRANULE
               jsr       >LC7BF              FIND FIRST FREE GRANULE IN FAT
               tfr       A,B                 SAVE FREE GRANULE NUMBER IN ACCB
               puls      A,U                 PULL LAST GRANULE POINTER AND COUNTER OFF OF STACK
               stb       ,U                  SAVE NEWLY FOUND GRANULE NUMBER IN ADDRESS OF LAST GRANULE
               deca                          DECREMENT GRANULE COUNTER
               bne       LC366               GET ANOTHER GRANULE IF NOT DONE
               pshs      X,B                 SAVE POINTER TO LAST GRANULE AND OFFSET
               jsr       >LC71E              WRITE FAT TO DISK
               puls      B,X                 RESTORE POINTER AND OFFSET
* WHEN CORRECT GRANULE IS FOUND, FIND THE RIGHT SECTOR
LC37B          leas      $01,S               REMOVE GRAN NUMBER FROM STACK
               ldu       FCBTMP              POINT U TO FCB
               stb       FCBCGR,U            SAVE CURRENT GRANULE IN FCB
               lda       #$FF                *SET FCBSOF,U TO ILLEGAL SECTOR OFFSET WHICH WILL
               sta       FCBSOF,U            *FORCE NEW SECTOR DATA TO BE READ IN
               lda       ,X                  GET CURRENT GRANULE
               cmpa      #$C0                IS IT THE LAST GRANULE?
               blo       LC3B2               NO
               anda      #$3F                MASK OFF LAST GRANULE FLAG BITS
               cmpa      FCBSEC,U            * COMPARE CALCULATED SECTOR TO CURRENT SECTOR IN FCB
               bhs       LC3B2               * AND BRANCH IF CALCULATED SECTOR IS > LAST SECTOR IN FILE
               lda       VD8                 = CHECK GET/PUT FLAG: IF 'GET' THEN 'INPUT
               beq       LC352               = PAST END OF FILE' ERROR
               lda       FCBSEC,U            * GET CURRENT SECTOR NUMBER FROM FCB,
               ora       #$C0                * OR IN THE LAST GRANULE FLAG BITS
               sta       ,X                  * AND SAVE IN FAT
               jsr       >LC5A9              WRITE FAT TO DISK IF NECESSARY
               ldx       FCBRLN,U            * GET RECORD LENGTH AND CHECK TO
               cmpx      #SECLEN             * SEE IF IT IS SECLEN (EXACTLY ONE SECTOR)
               bne       LC3AD               BRANCH IF IT IS NOT EXACTLY ONE SECTOR
               cmpx      FCBLST,U            =BRANCH IF THE NUMBER OF BYTES IN THE LAST SECTOR
               beq       LC3B2               =IS SET TO ONE SECTOR (SECLEN)
               lda       #$81                *SET THE PRESAVED FLAG (BIT15) AND FORCE
LC3AC          fcb       $21                 *THE NUMBER OF BYTES IN LAST SECTOR TO 256 (THROWN AWAY BRN INSTRUCTION)
LC3AD          clra                          SET	THE NUMBER OF BYTES IN LAST SECTOR TO ZERO
               clrb                          CLEAR	LS BYTE OF ACCD
               std       FCBLST,U            SAVE THE NUMBER OF BYTES IN LAST SECTOR
LC3B2          ldb       #$02                DSKCON READ OP CODE
               ldx       FCBRLN,U            * GET RECORD LENGTH AND COMPARE
               cmpx      #SECLEN             * IT TO SECLEN - EXACTLY ONE SECTOR
               bne       LC3C8               BRANCH IF NOT EXACTLY ONE SECTOR LONG
               leas      $07,S               CLEAN UP STACK
               ldx       FCBBUF,U            POINT X TO START OF RANDOM FILE BUFFER
               lda       VD8                 * CHECK GET/PUT FLAG AND
               beq       LC3C5               * BRANCH IF GET
               ldb       #$03                DSKCON WRITE OP CODE
LC3C5          jmp       >LC35A              READ/WRITE A SECTOR
LC3C8          jsr       >LC357              READ A SECTOR INTO FCB DATA BUFFER
               puls      B,X                 * GET BACK THE BYTE OFFSET TO RECORD: X = NUMBER OF
* * SECTORS; ACCB = BYTE POINTER IN SECTOR
               stx       FCBSOF,U            SAVE SECTOR OFFSET IN FCB
LC3CF          pshs      B                   SAVE BYTE OFFSET ON STACK
               jsr       >LC755              POINT X TO FILE ALLOCATION TABLE
               leax      FATCON,X            MOVE X TO FAT DATA
               ldb       FCBCGR,U            GET CURRENT GRANULE NUMBER
               abx                           POINT X TO PROPER GRANULE IN FAT
               lda       ,X                  * GET CURRENT GRANULE AND CHECK TO
               cmpa      #$C0                * SEE IF IT IS LAST GRANULE
               blo       LC40A               BRANCH IF THIS GRANULE IS < LAST GRANULE
               anda      #$3F                MASK OFF LAST GRANULE FLAG BITS
               cmpa      FCBSEC,U            * COMPARE LAST SECTOR USED IN GRANULE TO
               bne       LC40A               * CALCULATED SECTOR; BRANCH IF NOT EQUAL
               ldd       FCBLST,U            GET NUMBER OF BYTES IN LAST SECTOR
               anda      #$7F                MASK OFF PRESAVED FLAG (BIT 15)
               pshs      B,A                 SAVE NUMBER OF BYTES IN LAST SECTOR ON STACK
               clra                          *	LOAD ACCB WITH THE BYTE OFFSET TO CURRENT
               ldb       $02,S               * RECORD AND ADD THE REMAINING RECORD LENGTH
               addd      $03,S               * TO IT - ACCD = END OF RECORD OFFSET
               cmpd      ,S++                =COMPARE THE END OF RECORD OFFSET TO THE NUMBER OF
               bls       LC40A               =BYTES USED IN THE LAST SECTOR
               tst       VD8                 * CHECK GET/PUT FLAG AND BRANCH IF 'GET'
               lbeq      LC352               * TO 'INPUT PAST END OF FILE' ERROR
* IF LAST USED SECTOR, CALCULATE HOW MANY BYTES ARE USED
* IF DATA IS BEING 'PUT' PASTH THE CURRENT END OF FILE
               cmpd      #SECLEN             COMPARE TO ONE SECTOR'S LENGTH
               bls       LC405               BRANCH IF REMAINDER OF RECORD LENGTH WILL FIT IN THIS SECTOR
               ldd       #SECLEN             FORCE NUMBER OF BYTES = ONE SECTOR LENGTH
LC405          ora       #$80                * SET PRE-SAVED FLAG BIT - ALL PUT RECORDS ARE
* * WRITTEN TO DISK BEFORE LEAVING 'PUT'
               std       FCBLST,U            SAVE NUMBER OF BYTES USED IN LAST SECTOR
LC40A          puls      B                   PULL BYTE OFFSET OFF OF THE STACK
               leax      FCBCON,U            POINT X TO FCB DATA BUFFER
               abx                           MOVE X TO START OF RECORD
               ldu       $02,S               POINT U TO CURRENT POSITION IN RANDOM FILE BUFFER
               pshs      B                   SAVE BYTE OFFSET ON STACK
               lda       #-1                 * CONVERT ACCD INTO A NEGATIVE 2 BYTE NUMBER
* * REPRESENTING THE REMAINING UNUSED BYTES IN THE SECTOR
               addd      $01,S               * ADD TEMPORARY RECORD LENGTH COUNTER (SUBTRACT
* * REMAINING BYTES FROM TEMPORARY RECORD LENGTH)
               bhs       LC421               BRANCH IF THERE ARE ENOUGH UNUSED BYTES TO FINISH THE RECORD
               std       $01,S               SAVE NEW TEMPORARY RECORD LENGTH COUNTER
               puls      B                   RESTORE BYTE COUNTER
               negb                          * NEGATE IT - ACCB = THE NUMBER OF BYTES
* * AVAILABLE TO A RECORD IN THIS SECTOR
               bra       LC429               MOVE THE DATA
* BRANCH HERE IF REMAINING RECORD LENGTH WILL FIT IN
* WHAT'S LEFT OF THE CURRENTLY SELECTED SECTOR
LC421          ldb       $02,S               GET REMAINING RECORD LENGTH
               clr       $01,S               * CLEAR THE TEMPORARY RECORD LENGTH
               clr       $02,S               * COUNTER ON THE STACK
               leas      $01,S               PURGE BYTE OFFSET FROM STACK
LC429          lda       VD8                 * CHECK GET/PUT FLAG AND
               beq       LC42F               * BRANCH IF GET
               exg       X,U                 SWAP SOURCE AND DESTINATION POINTERS
LC42F          jsr       >LA59A              TRANSFER DATA FROM SOURCE TO DESTINATION BUFFERS
               stu       $02,S               SAVE NEW TEMP RECORD POINTER ON THE STACK (GET)
* MOVE DATA FROM FCB DATA BUFFER TO THE RANDOM FILE BUFFER IF 'GET'
* OR FROM RANDOM FILE BUFFER TO FCB DATA BUFFER IF 'PUT'
               ldu       FCBTMP              POINT U TO FCB
               lda       VD8                 * CHECK GET/PUT FLAG AND
               beq       LC43E               * BRANCH IF GET
               sta       FCBFLG,U            SAVE 'PUT' FLAG IN THE FCB
               stx       $02,S               SAVE NEW TEMPORARY RECORD POINTER ON STACK (PUT)
LC43E          ldx       FCBSOF,U            * GET SECTOR OFFSET COUNTER AND
               leax      $01,X               * ADD ONE TO IT
               clrb                          SET	BYTE OFFSET = 0
               ldu       ,S                  * CHECK THE LENGTH OF THE TEMPORARY RECORD LENGTH
               lbne      LC306               * COUNTER AND KEEP MOVING DATA IF <> 0
               puls      A,B,X,PC            * PULL TEMPORARY RECORD LENGTH AND
* * BUFFER ADDRESS OFF STACK AND RETURN
* OPEN RAM HOOK
DVEC0          leas      $02,S               PULL RETURN ADDRESS OFF OF THE STACK
               jsr       >LB156              EVALUATE AN EXPRESSION
               jsr       >LB6A4              *GET MODE(I,O,R) - FIRST BYTE OF STRING EXPRESSION
               pshs      B                   *AND SAVE IT ON STACK
               jsr       >LA5A2              GET DEVICE NUMBER
               tstb                          SET FLAGS
               lble      LA603               BRANCH IF NOT A DISK FILE
               puls      A                   GET MODE
               pshs      B,A                 SAVE MODE AND DEVICE NUMBER (FILE NUMBER)
               clr       DEVNUM              SET DEVICE NUMBER TO SCREEN
               jsr       SYNCOMMA            SYNTAX CHECK FOR COMMA
               ldx       #DATEXT             POINT TO 'DAT' FOR EXTENSION
               jsr       >LC938              GET FILENAME FROM BASIC
               ldd       #$01FF              DEFAULT DISK FILE TYPE AND ASCII FLAG
               std       DFLTYP              SAVE DEFAULT VALUES: DATA, ASCII
               ldx       #SECLEN             DEFAULT RECORD LENGTH - 1 PAGE
               jsr       GETCCH              GET CHAR FROM BASIC
               beq       LC481               BRANCH IF END OF LINE
               jsr       SYNCOMMA            SYNTAX CHECK FOR COMMA
               jsr       >LB3E6              EVALUATE EXPRESSION
               ldx       FPA0+2              GET EVALUATED EXPRESSION
LC481          stx       DFFLEN              RECORD LENGTH
               lbeq      LB44A               IF = 0, THEN 'ILLEGAL FUNCTION CALL'
               jsr       >LA5C7              ERROR IF ANY FURTHER CHARACTERS ON LINE
               puls      A,B                 GET MODE AND FILE NUMBER
* OPEN DISK FILE FOR READ OR WRITE
LC48D          pshs      A                   SAVE MODE ON STACK
               jsr       >LC749              POINT X TO FCB FOR THIS FILE
               lbne      LA61C               'FILE ALREADY OPEN' ERROR IF FILE OPEN
               stx       FCBTMP              SAVE FILE BUFFER POINTER
               jsr       >LC79D              MAKE SURE FILE ALLOC TABLE IS VALID
               jsr       >LC68C              SCAN DIRECTORY FOR 'FILENAME.EXT'
               puls      B                   GET MODE
               lda       #INPFIL             INPUT TYPE FILE
               pshs      A                   SAVE FILE TYPE ON STACK
               cmpb      #'I                 INPUT MODE?
               bne       LC4C7               BRANCH IF NOT
* OPEN A SEQUENTIAL FILE FOR INPUT
               jsr       >LC6E5              CHECK TO SEE IF DIRECTORY MATCH IS FOUND
               jsr       >LC807              CHECK TO SEE IF FILE ALREADY OPEN
               ldx       V974                GET RAM DIRECTORY BUFFER
               ldd       DIRTYP,X            GET FILE TYPE AND ASCII FLAG
               std       DFLTYP              SAVE IN RAM IMAGE
               bsr       LC52D               INITIALIZE FILE BUFFER CONTROL BLOCK
               jsr       >LC627              GO FILL DATA BUFFER
LC4BB          jsr       >LC755              POINT X TO PROPER FILE ALLOCATION TABLE
               inc       FAT0,X              ADD ONE TO FAT ACTIVE FILE COUNTER
               ldx       FCBTMP              GET FILE BUFFER POINTER
               puls      A                   GET FILE TYPE
               sta       FCBTYP,X            SAVE IT IN FCB
               rts       
LC4C7          asl       ,S                  SET FILE TYPE TO OUTPUT
               cmpb      #'O                 FILE MODE = OUTPUT?
               bne       LC4E8               BRANCH IF NOT
* OPEN A SEQUENTIAL FILE FOR OUTPUT
               tst       V973                DOES FILE EXIST ON DIRECTORY?
               beq       LC4E1               BRANCH IF NOT
               jsr       >LC6FC              KILL THE OLD FILE
               lda       V973                * GET DIRECTORY SECTOR NUMBER OF OLD FILE AND
               sta       V977                * SAVE IT AS FIRST FREE DIRECTORY ENTRY
               ldx       V974                =GET RAM DIRECTORY IMAGE OF OLD FILE AND
               stx       V978                =SAVE IT AS FIRST FREE DIRECTORY ENTRY
LC4E1          jsr       >LC567              SET UP NEW DIRECTORY ENTRY ON DISK
               bsr       LC538               INITIALIZE FILE BUFFER
               bra       LC4BB               FLAG AND MAP FCB AS BEING USED
LC4E8          cmpb      #'R                 FILE MODE = R (RANDOM)?
               beq       LC4F2               BRANCH IF SO
               cmpb      #'D                 FILE MODE = D (DIRECT)?
               lbne      LA616               'BAD FILE MODE' ERROR IF NOT
* OPEN A RANDOM/DIRECT FILE
LC4F2          asl       ,S                  SET FILE TYPE TO DIRECT
               ldd       RNBFAD              * GET ADDRESS OF RANDOM FILE BUFFER AREA
               pshs      B,A                 * AND SAVE IT ON THE STACK
               addd      DFFLEN              ADD THE RECORD LENGTH
               blo       LC504               'OB' ERROR IF SUM > $FFFF
               cmpd      FCBADR              IS IT > THAN FCB DATA AREA?
               bls       LC509               BRANCH IF NOT
LC504          ldb       #2*29               'OUT OF BUFFER SPACE' ERROR
               jmp       >LAC46              JUMP TO ERROR HANDLER
LC509          pshs      B,A                 SAVE END OF RANDOM BUFFER ON STACK
               tst       V973                DID THIS FILE EXIST
               bne       LC514               BRANCH IF SO
               bsr       LC567               SET UP NEW FILE IN DIRECTORY
               bra       LC519               INITIALIZE FCB
LC514          lda       #$FF                * SET FILE TYPE MATCH = $FF (ILLEGAL VALUE) -
               jsr       >LC807              * THIS WILL FORCE ANY OPEN MATCHED FILE TO CAUSE
* * A 'FILE ALREADY OPEN' ERROR
LC519          bsr       LC52D               INITIALIZE FCB
               com       FCBSOF,X            * SET FCBSOF,X TO $FF (ILLEGAL SECTOR OFFSET) WHICH WILL
* * FORCE NEW SECTOR DATA TO BE READ IN DURING GET/PUT
               inc       FCBREC+1,X          INITIALIZE RECORD NUMBER = 1
               puls      A,B,U               U = START OF RANDOM FILE BUFFER AREA, ACCD = END
               std       RNBFAD              SAVE NEW START OF RANDOM FILE BUFFER AREA
               stu       FCBBUF,X            SAVE BUFFER START IN FCB
               ldu       DFFLEN              * GET RANDOM FILE RECORD LENGTH
               stu       FCBRLN,X            * AND SAVE IT IN FCB
               bra       LC4BB               SET FAT FLAG, SAVE FILE TYPE IN FCB
* INITIALIZE FCB DATA FOR INPUT
LC52D          bsr       LC538               INITIALIZE FCB
               ldu       V974                GET RAM DIRECTORY IMAGE
               ldu       DIRLST,U            *GET NUMBER OF BYTES IN LAST SECTOR OF FILE
               stu       FCBLST,X            *SAVE IT IN FCB
               rts       
* INITIALIZE FILE CONTROL BLOCK
LC538          ldx       FCBTMP              GET CURRENT FILE BUFFER
               ldb       #FCBCON             CLEAR FCB CONTROL BYTES
LC53C          clr       ,X+                 CLEAR A BYTE
               decb                          DECREMENT COUNTER
               bne       LC53C               BRANCH IF NOT DONE
               ldx       FCBTMP              GET CURRENT FILE BUFFER ADDRESS BACK
               lda       DCDRV               *GET CURRENT DRIVE NUMBER AND
               sta       FCBDRV,X            *SAVE IT IN FCB
               lda       V976                =GET FIRST GRANULE -
               sta       FCBFGR,X            =SAVE IT AS THE STARTING GRANULE NUMBER AND
               sta       FCBCGR,X            =SAVE IT AS CURRENT GRANULE NUMBER
               ldb       V973                GET DIRECTORY SECTOR NUMBER
               subb      #$03                SUBTRACT 3 - DIRECTORY SECTORS START AT 3
               aslb                          * MULTIPLY SECTORS
               aslb                          * BY 8 (8 DIRECTORY
               aslb                          * ENTRIES PER SECTOR)
               pshs      B                   SAVE SECTOR OFFSET
               ldd       V974                GET RAM DIRECTORY IMAGE
               subd      #DBUF0              SUBTRACT RAM OFFSET
               lda       #$08                8 DIRECTORY ENTRIES/SECTOR
               mul                           NOW ACCA CONTAINS 0-7
               adda      ,S+                 ACCA CONTAINS DIRECTORY ENTRY (0-71)
               sta       FCBDIR,X            SAVE DIRECTORY ENTRY NUMBER
               rts       
* SET UP DIRECTORY AND UPDATE FILE ALLOCATION TABLE ENTRY IN FIRST UNUSED SECTOR
LC567          ldb       #28*2               'DISK FULL' ERROR
               lda       V977                GET SECTOR NUMBER OF FIRST EMPTY DIRECTORY ENTRY
               lbeq      LAC46               'DISK FULL' ERROR IF NO EMPTY DIRECTORY ENTRIES
               sta       V973                SAVE SECTOR NUMBER OF FIRST EMPTY DIRECTORY ENTRY
               sta       DSEC                SAVE SECTOR NUMBER IN DSKCON REGISTER
               ldb       #$02                READ OP CODE
               stb       DCOPC               SAVE IN DSKCON REGISTER
               jsr       >LD6F2              READ SECTOR
               ldx       V978                * GET ADDRESS OF RAM IMAGE OF UNUSED DIRECTORY
               stx       V974                * ENTRY AND SAVE AS CURRENT USED RAM IMAGE
               leau      ,X                  (TFR X,U) POINT U TO DIRECTORY RAM IMAGE
               ldb       #DIRLEN             SET COUNTER TO CLEAR 32 BYTES (DIRECTORY ENTRY)
LC586          clr       ,X+                 CLEAR BYTE
               decb                          DECREMENT COUNTER
               bne       LC586               CONTINUE IF NOT DONE
               ldx       #DNAMBF             POINT TO FILENAME AND EXTENSION RAM IMAGE
               ldb       #11                 11 BYTES IN FILENAME AND EXTENSION
               jsr       >LA59A              MOVE B BYTES FROM X TO U
               ldd       DFLTYP              GET FILE TYPE AND ASCII FLAG
               std       $00,U               SAVE IN RAM IMAGE
               ldb       #33                 FIRST GRANULE TO CHECK
               jsr       >LC7BF              FIND THE FIRST FREE GRANULE
               sta       V976                SAVE IN RAM
               sta       $02,U               SAVE IN RAM IMAGE OF DIRECTORY TRACK
               ldb       #$03                * GET WRITE OPERATION CODE AND SAVE
               stb       DCOPC               * IT IN DSKCON REGISTER
               jsr       >LD6F2              GO WRITE A SECTOR IN DIRECTORY
LC5A9          pshs      U,X,B,A             SAVE REGISTERS
               jsr       >LC755              POINT X TO FILE ALLOCATION TABLE
               inc       FAT1,X              INDICATE NEW DATA IN FILE ALLOC TABLE
               lda       FAT1,X              GET NEW DATA FLAG
               cmpa      WFATVL              * HAVE ENOUGH GRANULES BEEN REMOVED FROM THE FAT TO
* * CAUSE THE FAT TO BE WRITTEN TO THE DISK
               blo       LC5BA               RETURN IF NO NEED TO WRITE OUT ALLOCATION TABLE
               jsr       >LC71E              WRITE FILE ALLOCATION SECTOR TO DISK
LC5BA          puls      A,B,X,U,PC          RESTORE REGISTERS
* CONSOLE IN RAM VECTOR
DVEC4          lda       DEVNUM              GET DEVICE NUMBER
               lble      XVEC4               BRANCH IF NOT DISK FILE
               leas      $02,S               GET RID OF RETURN ADDRESS
LC5C4          pshs      X,B                 SAVE REGISTERS
               clr       CINBFL              CLEAR BUFFER NOT EMPTY FLAG
               ldx       #FCBV1-2            POINT TO FILE BUFFER VECTOR TABLE
               ldb       DEVNUM              GET ACTIVE DISK FILE NUMBER
               aslb                          TIMES 2 - TWO BYTES PER FCB ADDRESS
               ldx       B,X                 NOW X POINTS TO FILE BUFFER
               ldb       ,X                  GET FILE TYPE (FCBTYP,X)
               cmpb      #RANFIL             IS THIS A RANDOM (DIRECT) FILE?
               bne       LC5EC               BRANCH IF NOT
* GET A BYTE FROM A RANDOM FILE - RETURN CHAR IN ACCA
               ldd       FCBGET,X            GET THE RECORD COUNTER
               cmpd      FCBRLN,X            *COMPARE TO RECORD LENGTH AND
               bhs       LC5FE               *BRANCH TO BUFFER EMPTY IF >= RECORD LENGTH
               addd      #$0001              = ADD ONE TO RECORD POINTER AND
               std       FCBGET,X            = SAVE IT IN FCB
               ldx       FCBBUF,X            * POINT X TO START OF RANDOM FILE BUFFER AND
               leax      D,X                 * ADD THE RECORD COUNTER TO IT
               lda       $-1,X               GET A CHARACTER FROM THE BUFFER
               puls      B,X,PC              RESTORE REGISTERS AND RETURN
* GET A BYTE FROM A SEQUENTIAL FILE
LC5EC          ldb       FCBCFL,X            * TEST THE CACHE FLAG AND BRANCH IF AN
               beq       LC5F9               * EXTRA CHARACTER HAS NOT BEEN READ FROM FILE
               lda       FCBCDT,X            GET THE CACHE CHARACTER
               clr       FCBCFL,X            CLEAR THE CACHE FLAG
               puls      B,X,PC              RESTORE REGISTERS AND RETURN
LC5F9          ldb       FCBDFL,X            IS ANY DATA LEFT?
               beq       LC602               BRANCH IF SO
LC5FE          com       CINBFL              SET FLAG TO BUFFER EMPTY
               puls      B,X,PC              RESTORE REGISTERS AND RETURN
LC602          ldb       FCBCPT,X            GET CHARACTER POINTER
               inc       FCBCPT,X            ADD ONE TO CHARACTER POINTER
               dec       FCBLFT,X            DECREMENT NUMBER OF CHARACTERS LEFT IN FILE BUFFER
               beq       LC611               IF LAST CHARACTER, GO GET SOME MORE
               abx                           ADD CHARACTER COUNTER TO X
               lda       FCBCON,X            GET DATA CHARACTER (SKIP PAST 25 FCB CONTROL BYTES
               puls      B,X,PC
* GET A CHARACTER FROM FCB DATA BUFFER - RETURN CHAR IN ACCA
LC611          pshs      U,Y                 SAVE REGISTERS
               clra                          *
               leau      D,X                 * POINT U TO CORRECT CHARACTER
               lda       FCBCON,U            =GET DATA CHAR (SKIP PAST 25 CONTROL BYTES)
               pshs      A                   =AND SAVE DATA CHARACTER ON STACK
               clr       FCBCPT,X            RESET CHAR POINTER TO START OF BUFFER
               lda       FCBDRV,X            GET DRIVE NUMBER AND SAVE IT IN
               sta       DCDRV               DSKCON VARIABLE
               bsr       LC627               GO READ A SECTOR - FILL THE BUFFER
               puls      A,Y,U               RESTORE REGISTERS AND DATA CHARACTER
               puls      B,X,PC              RESTORE REGISTERS AND RETURN
* REFILL THE FCB INPUT DATA BUFFER FOR SEQUENTIAL FILES
LC627          lda       FCBSEC,X            GET CURRENT SECTOR NUMBER
LC629          inca                          ADD ONE
               pshs      A                   SAVE NEW SECTOR NUMBER ON THE STACK
               cmpa      #$09                NINE SECTORS PER GRANULE
               bls       LC631               BRANCH IF <= 9
               clra                          SET	TO SECTOR ZERO
LC631          sta       FCBSEC,X            SAVE SECTOR NUMBER
               ldb       FCBCGR,X            GET GRANULE NUMBET TO FAT POINTER
               leau      ,X                  POINT U TO FCB (TFR X,U)
               jsr       >LC755              POINT X TO PROPER FILE ALLOCATION TABLE
               abx                           ADD OLD GRANULE NUMBER TO FAT POINTER
               ldb       FATCON,X            GET GRANULE NUMBER (6 CONTROL BYTES AT FRONT OF FAT)
               leax      ,U                  POINT X TO FCB
               cmpb      #$C0                IS CURRENT GRANULE LAST ONE IN FILE?
               bhs       LC64D               YES
               puls      A                   GET SECTOR NUMBER
               suba      #10                 WAS IT 10? - OVERFLOW TO NEXT GRANULE IF SO
               bne       LC65E               BRANCH IF NOT
               stb       FCBCGR,X            SAVE NEW GRANULE NUMBER
               bra       LC629               SET VARIABLES FOR NEW GRANULE
LC64D          andb      #$3F                GET NUMBER OF SECTORS USED IN THIS GRANULE
               cmpb      #$09                9 SECTORS / GRANULE
               bls       LC658               BRANCH IF OK
LC653          ldb       #2*32               'BAD FILE STRUCTURE' ERROR
               jmp       >LAC46              ERROR DRIVER
LC658          subb      ,S+                 SUBTRACT CURRENT SECTOR NUMBER AND PULS A
               blo       LC67D               BRANCH IF PAST LAST SECTOR
               tfr       B,A                 SECTOR NUMBER TO ACCA
LC65E          pshs      A                   SAVE SECTOR NUMBER DIFFERENCE
               bsr       LC685               INCREMENT RECORD NUMBER
               lda       #$02                *GET READ OPERATION CODE
               sta       DCOPC               *AND SAVE IT IN DSKCON VARIABLE
               jsr       >LC763              GET PROPER TRACK AND SECTOR TO DSKCON VARIABLES
               leau      FCBCON,X            * POINT U TO START OF FCB DATA BUFFER
               stu       DCBPT               * AND SAVE IT IN DSKCON VARIABLE
               jsr       >LD6F2              GO READ A SECTOR INTO FCB BUFFER
               clr       FCBLFT,X            NUMBER OF CHARS LEFT IN BUFFER = 256
               ldb       ,S+                 GET SECTOR NUMBER OFF STACK
               bne       LC684               RETURN IF DATA LEFT; FALL THRU IF LAST SECTOR
               ldd       FCBLST,X            GET NUMBER OF BYTES IN THE LAST SECTOR
               bne       LC681               BRANCH IF SOME BYTES IN LAST SECTOR
LC67D          clrb                          SET	NUMBER OF REMAINING BYTES = 256
               com       FCBDFL,X            SET DATA LEFT FLAG TO $FF
LC681          stb       FCBLFT,X            SAVE THE NUMBER OF CHARS LEFT IN BUFFER
LC684          rts       
LC685          ldu       FCBREC,X            GET CURRENT RECORD NUMBER
               leau      $01,U               BUMP IT
               stu       FCBREC,X            PUT IT BACK
               rts       
* SCAN DIRECTORY FOR FILENAME.EXT FOUND IN DNAMBF. IF FILENAME FOUND,
* RETURN WITH SECTOR NUMBER IN V973, GRANULE IN V976 AND RAM BUFFER
* CONTAINING DIRECTORY DATA IN V974. IF DISK IS FULL THEN V973,
* V977 = 0. THE FIRST UNUSED SECTOR RETURNED IN V977, RAM IMAGE IN V978
LC68C          clr       V973                CLEAR SECTOR NUMBER
               clr       V977                CLEAR TEMP SECTOR COUNTER
               ldd       #$1102              TRACK 17 (DIRECTORY), READ OPERATION CODE
               sta       DCTRK               SAVE TRACK NUMBER
               stb       DCOPC               SAVE OPERATION CODE (READ)
               ldb       #$03                READ SECTOR 3 (FIRST DIRECTORY SECTOR)
LC69B          stb       DSEC                SAVE SECTOR NUMBER IN DSKCON VARIABLE
               ldu       #DBUF0              *BUFFER AREA NUMBER 0 AS DATA BUFFER - SAVE
               stu       DCBPT               *IN DSKCON VARIABLE
               jsr       >LD6F2              GO READ A SECTOR
LC6A5          stu       V974                SAVE RAM DIRECTORY BUFFER ADDRESS
               leay      ,U                  POINT Y TO DIRECTORY BUFFER
               lda       ,U                  GET A BYTE FROM BUFFER
               bne       LC6D6               BRANCH IF NOT ZERO - FILE IS ACTIVE
               bsr       LC6D9               SET UNUSED FILE POINTERS IF ENTRY HAS BEEN KILLED
LC6B0          ldx       #DNAMBF             POINT TO DISK FILE NAME BUFFER
LC6B3          lda       ,X+                 *COMPARE THE FILENAME AND EXTENSION
               cmpa      ,U+                 *STORED IN RAM AT DNAMBF TO THE DIRECTORY
               bne       LC6C7               *ENTRY STORED AT ,U (BRANCH IF MISMATCH)
               cmpx      #DNAMBF+11          AT END OF FILE NAME BUFFER?
               bne       LC6B3               BRANCH IF NOT DONE CHECKING FILENAME
               stb       V973                SAVE SECTOR NUMBER IN DSKCON VARIABLE
               lda       FCBFGR,U            *GET NUMBER OF FIRST GRANULE IN FILE
               sta       V976                *AND SAVE IT IN V976
               rts       
LC6C7          leau      DIRLEN,Y            GET NEXT DIRECTORY ENTRY (DIRLEN BYTES PER ENTRY)
               cmpu      #DBUF0+SECLEN       AT END OF BUFFER?
               bne       LC6A5               CHECK NEXT ENTRY IF NOT AT END
               incb                          NEXT SECTOR
               cmpb      #11                 11 SECTORS MAX IN DIRECTORY
               bls       LC69B               BRANCH IF MORE SECTORS
               rts       
LC6D6          coma                          COMPLEMENT FIRST BYTE IN DIRECTORY EMTRY
               bne       LC6B0               BRANCH IF FILE IS ACTIVE - FALL THRU IF NOT USED
* SET POINTERS FOR FIRST UNUSED DIRECTORY ENTRY
LC6D9          lda       V977                UNUSED ENTRY ALREADY FOUND?
               bne       DVEC12              RETURN IF UNUSED ENTRY ALREADY FOUND
               stb       V977                SECTOR CONTAINING THIS DIRECTORY ENTRY
               stu       V978                POINTS TO RAM AREA WHERE DIRECTORY DATA IS STORED
DVEC12         rts       
LC6E5          ldb       #2*26               'NE' ERROR
               tst       V973                WAS A DIRECTORY MATCH FOUND?
               bne       DVEC12              RETURN IF FOUND
               jmp       >LAC46              JUMP TO ERROR HANDLER IF NOT FOUND
* KILL COMMAND
KILL           jsr       >LC935              GET FILENAME.EXT FROM BASIC
               jsr       >LA5C7              'SYNTAX' ERROR IF MORE CHARACTERS ON LINE
LC6F5          jsr       >LC79D              GET VALID FAT DATA
               bsr       LC68C               TEST FOR FILE NAME MATCH IN DIRECTORY
               bsr       LC6E5               MAKE SURE THE FILE EXISTED
LC6FC          lda       #$FF                * MATCH FILE TYPE = $FF; THIS WILL CAUSE AN 'AO'
*			* ERROR TO BE GENERATED IF ANY FILE TYPE IS OPEN
               jsr       >LC807              CHECK TO MAKE SURE FILE IS NOT OPEN
               ldx       V974                *GET RAM IMAGE OF DIRECTORY
               clr       ,X                  *AND ZERO FIRST BYTE - KILL FILE (DIRNAM,X)
               ldb       #$03                =WRITE OPERATION CODE - SAVE
               stb       DCOPC               =IT IN DSKCON VARIABLE
               jsr       >LD6F2              WRITE A SECTOR
               ldb       DIRGRN,X            GET NUMBER OF FIRST GRANULE IN FILE
LC70F          bsr       LC755               POINT X TO PROPER FILE ALLOCATION TABLE
               leax      FATCON,X            SKIP 6 CONTROL BYTES
               abx                           POINT TO CORRECT ENTRY
               ldb       ,X                  GET NEXT GRANULE
               lda       #$FF                *GET FREE GRANULE FLAG AND
               sta       ,X                  *MARK GRANULE AS FREE
               cmpb      #$C0                WAS THIS THE LAST GRANULE?
               blo       LC70F               * KEEP FREEING GRANULES IF NOT LAST ONE
* * WRITE FILE ALLOCATION SECTOR TO DIRECTORY - DO NOT WRITE
* * THE SIX CONTROL BYTES AT THE START OF THE FAT TO THE DISK
LC71E          ldu       #DBUF0              =POINT U TO DISK BUFFER 0 AND
               stu       DCBPT               =SAVE IT AS DSKCON VARIABLE
               ldd       #$1103              * WRITE DIRECTORY TRACK - SAVE
               sta       DCTRK               * TRACK AND WRITE OPERATION CODE IN
               stb       DCOPC               * DSKCON VARIABLES
               ldb       #$02                = GET FILE ALLOCATION SECTOR AND
               stb       DSEC                = SAVE IN DSKCON VARIABLE
               bsr       LC755               POINT X TO PROPER FILE ALLOCATION TABLE
               clr       FAT1,X              RESET FLAG INDICATING VALID FAT DATA HAS BEEN STORED ON DISK
               leax      FATCON,X            MOVE (X) TO START OF GRANULE DATA
               ldb       #GRANMX             68 BYTES IN FAT
               jsr       >LA59A              MOVE ACCB BYTES FROM FAT RAM IMAGE TO DBUF0
* ZERO OUT ALL OF THE BYTES IN THE FAT SECTOR WHICH DO NOT CONTAIN THE GRANULE DATA
LC739          clr       ,U+                 CLEAR A BYTE
               cmpu      #DBUF0+SECLEN       FINISHED THE WHOLE SECTOR?
               bne       LC739               NO
               jmp       >LD6F2              WRITE A SECTOR
* ENTER WITH ACCB CONTAINING FILE NUMBER (1-15); EXIT WITH X POINTING
* TO CORRECT FILE BUFFER; FLAGS SET ACCORDING TO FILE TYPE.
LC744          pshs      B                   SAVE FILE NUMBER ON STACK
               ldb       DEVNUM              GET DEVICE NUMBER (FILE NUMBER)
               fcb       $8C                 SKIP TWO BYTES (THROWN AWAY CMPX INSTRUCTION)
LC749          pshs      B                   SAVE FILE NUMBER ON STACK
               aslb                          X2: 2 BYTES PER POINTER
               ldx       #FCBV1-2            POINT X TO START OF FCB POINTERS
               ldx       B,X                 POINT X TO PROPER FCB
               ldb       FCBTYP,X            SET FLAGS ACCORDING TO FILE TYPE
               puls      B,PC                RESTORE FILE NUMBER
* POINT X TO DRIVE ALLOCATION TABLE
LC755          jmp       >GETFAT

* Use only 2 Track tables for DS drives (no re-org)
TRKTBL         ldx       #$97E               Point to track tables
               ldb       <DCDRV              Get drive number
               andb      #1                  Use only 0 or 1
               rts       
               nop       
               nop       
               nop       

* LC755	PSHS	B,A	SAVE ACCD ON STACK
*	LDA	DCDRV	GET DRIVE NUMBER
*	LDB	#FATLEN	GET LENGTH OF FILE ALLOCATION TABLE
*	MUL		MULTIPLY BY DRIVE NUMBER TO GET OFFSET
*	LDX	#FATBL0	START OF FILE ALLOCATION TABLE
*	LEAX	D,X	POINT TO RIGHT TABLE
*	PULS	A,B,PC	RESTORE ACCD
* CONVERT GRANULE NUMBER TO TRACK & SECTOR NUMBER - X MUST BE POINTING TO CORRECT
* FCB; THE TRACK AND SECTOR NUMBER WILL BE STORED IN DSKCON REGISTERS
LC763          ldb       FCBCGR,X            GET GRANULE NUMBER
               lsrb                          DIVIDE BY 2 - 2 GRANULES / TRACK
               stb       DCTRK               TRACK NUMBER
               cmpb      #17                 TRACK 17 = DIRECTORY TRACK
               blo       LC76E               BRANCH IF < DIRECTORY TRACK
               inc       DCTRK               INCR TRACK NUMBER IF > DIRECTORY TRACK
LC76E          aslb                          MULTIPLY TRACK NUMBER BY 2
               negb                          NEGATE GRANULE NUMBER
               addb      FCBCGR,X            B=0 IF EVEN GRANULE; 1 IF ODD
               bsr       LC779               RETURN B=0 FOR EVEN GRANULE NUMBER, B=9 FOR ODD GRANULE NUMBER
               addb      FCBSEC,X            ADD SECTOR NUMBER
               stb       DSEC                SAVE SECTOR NUMBER
               rts       
* MULTIPLY ACCD BY 9
LC779          pshs      B,A                 TEMP STORE ACCD ON STACK
               aslb                          *
               rola                          * MULTIPLY BY 2
               aslb                          =
               rola                          = MULTIPLY BY FOUR
               aslb                          *
               rola                          * MULTIPLY BY EIGHT
               addd      ,S++                ADD ONE = MULTIPLY BY NINE
               rts       
* CONVERT ACCD INTO A GRANULE NUMBER - RETURN RESULT IN ACCB;
* ENTER WITH ACCD CONTAINING A NUMBER OF SECTORS. RETURN IN ACCB
* THE NUMBER (0-67) CORRESPONDING TO THE NUMBER OF COMPLETE
* GRANULES CONTAINED IN THAT MANY SECTORS.
* DIVIDE BY 90, MULTIPLY BY 10 IS FASTER THAN DIVIDE BY 9
LC784          clr       ,-S                 CLEAR A TEMPORARY SLOT ON THE STACK
LC786          inc       ,S                  * DIVIDE ACCD BY 90 - SAVE THE
               subd      #9*10               * QUOTIENT+1 ON THE STACK - REMAINDER
               bpl       LC786               * IN ACCB
               lda       ,S                  = PUT THE QUOTIENT+1 IN ACCA AND
               stb       ,S                  = SAVE REMAINDER ON STACK
               ldb       #10                 * MULTIPLY (QUOTIENT+1)
               mul                           * BY 10
               puls      A                   PUT THE REMAINDER IN ACCA
LC796          decb                          * DECREMENT THE GRANULE COUNT BY ONE FOR
               adda      #$09                * EVERY NINE SECTORS (1 GRANULE) IN THE
               bmi       LC796               * REMAINDER - COMPENSATE FOR THE + 1 IN QUOTIENT+1
               clra                          CLEAR	MS BYTE OF ACCD
LC79C          rts       
* MAKE SURE RAM FILE ALLOCATION TABLE DATA IS VALID
LC79D          bsr       LC755               POINT X TO FAT FOR THE CORRECT DRIVE NUMBER
               tst       FAT0,X              CHECK TO SEE IF ANY FILES ARE ACTIVE
               bne       LC79C               RETURN IF ANY FILES ACTIVE IN THIS FAT
               clr       FAT1,X              RESET FAT DATA VALID FLAG
               leau      FATCON,X            LOAD U WITH START OF GRANULE DATA BUFFER
               ldx       #DBUF0              BUFFER FOR DISK TRANSFER
               stx       DCBPT               PUT IN DSKCON PARAMETER
               ldd       #$1102              DIRECTORY TRACK, READ SECTOR
               sta       DCTRK               STORE IN DSKCON TRACK NUMBER
               stb       DCOPC               STORE IN DSKCON OP CODE
               ldb       #$02                GET SECTOR NUMBER 2 (FILE ALLOCATION TABLE)
               stb       DSEC                STORE IN DSKCON PARAMETER
               jsr       >LD6F2              GO READ SECTOR
               ldb       #GRANMX             TRANSFER FILE ALLOCATION TABLE TO FILE ALLOC TABLE BUFFER
               jmp       >LA59A              MOVE B BYTES FROM (X) TO (U)
* FIND FIRST FREE GRANULE - ENTER WITH ACCB CONTAINING
* GRANULE FROM WHICH TO START SEARCHING. THE FOUND GRANULE
* IS MARKED BY STORING A $C0 IN THE GRANULE'S DATA BYTE
* TO INDICATE THAT IT IS THE LAST GRANULE IN THE FILE.
* RETURN WITH FIRST FREE GRANULE FOUND IN ACCA
LC7BF          bsr       LC755               POINT X TO FILE ALLOC TABLE
               leax      FATCON,X            SKIP CONTROL BYTES
               clra                          USE	ACCA AS GRANULE COUNTER
               andb      #$FE                MASK OFF BIT ZERO OF SEARCH GRANULE
               clr       ,-S                 INITIALIZE AND SAVE A BYTE ON STACK (DIRECTION FLAG)
LC7C8          com       B,X                 IS THIS GRANULE FREE? ($FF=FREE)
               beq       LC7FD               BRANCH IF IT IS
               com       B,X                 RESTORE GRANULE DATA
               inca                          ADD ONE TO GRANULE COUNTER
               cmpa      #GRANMX             GRANMX GEANULES PER DISK
               bhs       LC7F8               BRANCH IF ALL GRANULES CHECKED (DISK FULL)
               incb                          INCR TO NEXT GRANULE
               bitb      #$01                IS BIT 0 SET?
               bne       LC7C8               BRANCH IF ODD GRANULE NUMBER (SAME TRACK)
               pshs      B,A                 SAVE GRANULE COUNTER AND CURRENT GRANULE NUMBER
               subb      #$02                SUBTRACT ONE TRACK (2 GRANULES)
               com       $02,S               COMPLEMENT DIRECTION FLAG
               bne       LC7EC               BRANCH EVERY OTHER TIME
               subb      ,S+                 SUBTRACT THE GRANULE COUNTER FROM THE CURRENT GRANULE NUMBER
               bpl       LC7E8               BRANCH IF LOWER BOUND NOT EXCEEDED
               ldb       ,S                  RESTORE CURRENT GRANULE NUMBER IF LOWER BOUND EXCEEDED
LC7E6          com       $01,S               * COMPLEMENT FLAG - IF GRANULE NUMBER HAS EXCEEDED
* * BOUNDS ON EITHER THE HI OR LO SIDE, FORCE IT TO GO IN
* * THE DIRECTION OPPOSITE THE EXCEEDED BOUND
LC7E8          leas      $01,S               CLEAN UP STACK
               bra       LC7C8               CHECK FOR ANOTHER FREE GRANULE
LC7EC          addb      ,S+                 ADD THE GRANULE COUNTER TO THE CURRENT GRANULE NUMBER
               cmpb      #GRANMX             GRANMX GRANULES PER DISK
               blo       LC7E8               BRANCH IF UPPER BOUND NOT EXCEEDED
               ldb       ,S                  * RESTORE CURRENT GRANULE COUNT AND GO TWICE
               subb      #$04                * AS FAR AS USUAL IN OPPOSITE DIRECTION IF UPPER BOUND EXCEEDED
               bra       LC7E6               KEEP SEARCHING
LC7F8          ldb       #2*28               'DISK FULL' ERROR
               jmp       >LAC46              JUMP TO ERROR HANDLER
* POINT X TO FIRST FREE GRANULE POSITION IN THE FILE ALLOCATION
* TABLE AND MARK THE POSITION WITH A LAST GRANULE IN FILE MARKER
LC7FD          leas      $01,S               CLEAR UP STACK - REMOVE DIRECTION FLAG
               tfr       B,A                 GRANULE NUMBER TO ACCA
               abx                           POINT X TO FIRST FOUND GRANULE
               ldb       #$C0                LAST GRANULE FLAG
               stb       ,X                  MARK THE FIRST FOUND GRANULE AS THE LAST GRANULE
LC806          rts       
* CHECK ALL ACTIVE FILES TO MAKE SURE A FILE IS NOT ALREADY OPEN - TO BE OPEN
* A FILE BUFFER MUST MATCH THE DRIVE NUMBER AND FIRST GRANULE NUMBER
* IN RAM DIRECTORY ENTRY AND THE FCB TYPE MUST NOT MATCH THE FILE TYPE IN ACCA
* AN 'AO' ERROR WILL NOT BE GENERATED IF A FILE IS BEING OPENED FOR
* THE SAME MODE THAT IT HAS ALREADY BEEN OPENED UNDER.
LC807          pshs      A                   SAVE FILE TYPE ON STACK
               ldb       FCBACT              NUMBER OF CURRENTLY OPEN FILES
               incb                          ADD ONE MORE TO FILE COUNTER
LC80D          jsr       >LC749              POINT X TO FCB OF THIS FILE
               beq       LC829               BRANCH IF BUFFER NOT BEING USED
               lda       DCDRV               * GET DRIVE NUMBER AND CHECK TO SEE IF IT
               cmpa      FCBDRV,X            * MATCHES THE DRIVE NUMBER FOR THIS BUFFER
               bne       LC829               FILE EXISTS ON ANOTHER DRIVE
               ldu       V974                GET RAM DIRECTORY AREA
               lda       DIRGRN,U            GET FIRST GRANULE IN FILE
               cmpa      FCBFGR,X            DOES IT MATCH THIS FILE BUFFER?
               bne       LC829               NO
               lda       FCBTYP,X            GET FILE TYPE OF THIS BUFFER
               cmpa      ,S                  DOES IT MATCH THE ONE WE ARE LOOKING FOR?
               lbne      LA61C               'FILE ALREADY OPEN' ERROR IF NOT
LC829          decb                          DECR FILE COUNTER
               bne       LC80D               BRANCH IF HAVEN'T CHECKED ALL ACTIVE FILES
               puls      A,PC                RESTORE FILE TYPE AND RETURN
LC82E          jsr       >LA5A5              EVALUATE AN EXPRESSION (DEVICE NUMBER)
               clr       DEVNUM              SET DEVICE NUMBER TO SCREEN
               tstb                          TEST NEW DEVICE NUMBER
               lble      LB44A               'FC' ERROR IF DEVICE NUMBER NOT A DISK FILE
               jsr       >LC749              POINT X TO FCB
               lda       FCBTYP,X            TEST IF BUFFER IS IN USE
               lbeq      LA3FB               'FILE NOT OPEN' ERROR
               cmpa      #RANFIL             DIRECT/RANDOM FILE?
               beq       LC806               RETURN IF RANDOM
LC845          jmp       >LA616              BAD FILE MODE ERROR IF NOT RANDOM
* INPUT DEVICE NUMBER CHECK RAM HOOK
DVEC5          lda       #INPFIL             INPUT FILE TYPE
LC84A          fcb       $8C                 SKIP TWO BYTES (THROWN AWAY CMPX INSTRUCTION)
* PRINT DEVICE NUMBER CHECK RAM HOOK
DVEC6          lda       #OUTFIL             OUTPUT FILE TYPE
               tst       DEVNUM              * CHECK DEVICE NUMBER AND RETURN IF
               ble       LC806               * NOT A DISK FILE
               stx       ,S                  = REPLACE SUBROUTINE RETURN ADDRESS WITH X REGISTER -
* = THIS IS THE SAME AS LEAS 2,S AND PSHS X
               jsr       >LC744              POINT X TO FCB
               pshs      B,A                 SAVE ACCB AND FILE TYPE ON STACK
               lda       FCBTYP,X            GET FILE TYPE
               lbeq      LA3FB               'FILE NOT OPEN' ERROR
               cmpa      #RANFIL             RANDOM FILE?
               beq       LC868               BRANCH IF RANDOM FILE
               cmpa      ,S                  IS THIS FCB OF THE PROPER TYPE?
               bne       LC845               'FILE MODE' ERROR IF NOT
LC866          puls      A,B,X,PC            RESTORE ACCB,X,ACCA (FILE TYPE) AND RETURN
LC868          ldx       $04,S               * GET CALLING ADDRESS FROM THE STACK AND
               cmpx      #LB00C              * RETURN UNLESS COMING FROM
               bne       LC866               * BASIC'S 'INPUT' STATEMENT
               jsr       SYNCOMMA            SYNTAX CHECK FOR A COMMA
               cmpa      #'"                 CHECK FOR A DOUBLE QUOTE
               bne       LC881               RETURN TO BASIC'S 'INPUT' COMMAND
               jsr       >LB244              STRIP PROMPT STRING FROM BASIC AND PUT IT ON THE STRING STACK
               jsr       >LB657              PURGE THE STRING PUT ON THE STRING STACK
               ldb       #';                 SEMICOLON
               jsr       >LB26F              DO A SYNTAX CHECK FOR SEMICOLON
LC881          ldx       #LB01E              GET MODIFIED REENTRY POINT INTO BASIC
               stx       $04,S               AND PUT IT INTO THE RETURN ADDRESS ON THE STACK
               puls      A,B,X,PC            RETURN TO BASIC
* DEVICE NUMBER VALIDITY CHECK RAM HOOK
DVEC1          ble       LC8AF               RETURN IF NOT A DISK FILE
               cmpb      FCBACT              COMPARE DEVICE NUMBER TO HIGHEST POSSIBLE
               lbhi      LA61F               'DEVICE NUMBER' ERROR IF TOO BIG
               puls      X,PC                RETURN
* SET PRINT PARAMETERS RAM HOOK
DVEC2          tst       DEVNUM              *CHECK DEVICE NUMBER AND
               ble       LC8AF               *RETURN IF NOT DISK FILE
               leas      $02,S               PURGE RETURN ADDRESS OFF OF THE STACK
               pshs      X,B,A               SAVE REGISTERS
               clr       PRTDEV              SET PRINT DEVICE NUMBER TO NON-CASSETTE
               jsr       >LC744              POINT X TO FCB
               ldb       FCBPOS,X            GET PRINT POSITION
               clra                          PRINT	WIDTH (256)
               ldx       #$1000              TAB FIELD WIDTH AND TAB ZONE
               jmp       >LA37C              SAVE THE PRINT PARAMETERS
* BREAK CHECK RAM HOOK
DVEC11         tst       DEVNUM              * CHECK DEVICE NUMBER AND RETURN
               ble       LC8AF               * IF NOT A DISK FILE
               leas      $02,S               = PURGE RETURN ADDRESS OFF OF THE STACK - DON'T
LC8AF          rts                           = DO A BREAK CHECK IF DISK FILE
* COMMAND INTERPRETATION RAM HOOK
DVEC20         leas      $02,S               PURGE RETURN ADDRESS OFF OF THE STACK
LC8B2          andcc     #$AF                ENABLE IRQ & FIRQ
               clr       $FF02
               lda       $FF00
               coma      
               anda      #$7F
               beq       LC8C2

*	CLR	PIA0+2	STROBE ALL KEYS (COLUMN STROBE)
*	LDA	PIA0	READ KEYBOARD ROWS
*	COMA		INVERT KEYBOARD ROW DATA
*	ANDA	#$7F	MASK OFF JOYSTICK INPUT BIT
*	BEQ	LC8C2	BRANCH IF NO KEY DOWN
               jsr       >LADEB              GO DO A BREAK CHECK IF A KEY IS DOWN
LC8C2          ldx       CHARAD              GET INPUT POINTER INTO X
               stx       TINPTR              TEMP SAVE IT
               lda       ,X+                 SEARCH FOR THE END OF CURRENT LINE
               beq       LC8D1               BRANCH IF END OF LINE
               cmpa      #':                 CHECK FOR END OF SUB LINE, TOO
               beq       LC8F3               BRANCH IF END OF SUB LINE
               jmp       >LB277              'SYNTAX' ERROR IF NOT END OF LINE
LC8D1          lda       ,X++                *GET MS BYTE OF ADDRESS OF NEXT BASIC LINE
               sta       ENDFLG              *AND SAVE IT IN CURLIN
               bne       LC8DA               BRANCH IF NOT END OF PROGRAM
               jmp       >LAE15              GO 'STOP' THE SYSTEM
LC8DA          ldd       ,X+                 *GET LINE NUMBER OF THIS LINE AND
               std       CURLIN              *SAVE IT IN CURLIN
               stx       CHARAD              RESET BASIC'S INPUT POINTER
               lda       TRCFLG              * CHECK THE TRACE FLAG AND
               beq       LC8F3               * BRANCH IF TRACE OFF
               lda       #'[                 [ LEFT DELIMITER OF TRON
               jsr       PUTCHR              SEND CHARACTER TO CONSOLE OUT
               lda       CURLIN              GET NUMBER OF CURRENT LINE NUMBER
               jsr       >LBDCC              CONVERT ACCD TO DECIMAL & PRINT IT ON SCREEN
               lda       #']                 ] RIGHT DELIMITER OF TRON
               jsr       PUTCHR              SEND A CHARACTER TO CONSOLE OUT
LC8F3          jsr       GETNCH              GET NEXT CHARACTER FROM BASIC
               tfr       CC,B                SAVE STATUS REGISTER IN ACCB
               cmpa      #TOKEN_CSAVE        CSAVE TOKEN?
               bne       LC8FE               NO
               jmp       >L8316              GO CHECK FOR CSAVEM
LC8FE          cmpa      #TOKEN_CLOAD        CLOAD TOKEN?
               bne       LC905               NO
               jmp       >L8311              JUMP TO EXBAS' CLOAD ROUTINE
LC905          tfr       B,CC                RESTORE STATUS REGISTER
               jsr       >LADC6              LOOP THROUGH BASIC'S MAIN INTERPRETATION LOOP
               bra       LC8B2
* EOF RAM HOOK
DVEC14         leas      $02,S               PURGE RETURN ADDRESS OFF OF THE STACK
               lda       DEVNUM              * GET DEVICE NUMBER AND SAVE
               pshs      A                   * IT ON THE STACK
               jsr       >LA5AE              STRIP DEVICE NUMBER OFF OF INPUT LINE
               jsr       >LA3ED              VERIFY THAT THE FILE TYPE WAS 'INPUT'
               tst       DEVNUM              * CHECK DEVICE NUMBER AND
               lble      LA5DA               * BRANCH BACK TO BASIC'S EOF IF NOT DISK FILE
               jsr       >LC744              POINT X TO FCB
               ldb       FCBTYP,X            GET FILE TYPE
               cmpb      #RANFIL             RANDOM FILE?
               lbeq      LA616               'FM' BAD FILE MODE ERROR IF RANDOM
               clrb                          FILE	NOT EMPTY FLAG - SET TO NOT EMPTY
               lda       FCBCFL,X            *CHECK THE CACHE FLAG - BRANCH IF
               bne       LC932               *THERE IS A CHARACTER WHICH HAS BEEN CACHED
               ldb       FCBDFL,X            GET SEQUENTIAL INPUT FILE STATUS
LC932          jmp       >LA5E4              LINK BACK TO BASIC'S EOF STATEMENT
* GET FILENAME/EXTENSION: DRIVE NUMBER FROM BASIC
LC935          ldx       #DEFEXT             POINT TO ' ' BLANK (DEFAULT) EXTENSION
LC938          clr       ,-S                 CLEAR A BYTE ON STACK FOR USE AS A DRIVES FLAG
               lda       DEFDRV              * GET DEFAULT DISK NUMBER
               sta       DCDRV               * STORE IN DSKCON PARAMETER
               ldu       #DNAMBF             DISK FILENAME BUFFER
               ldd       #$2008              STORE 8 BLANKS IN RAM (DEFAULT FILE NAME)
LC945          sta       ,U+                 STORE A BLANK IN FILE NAME
               decb                          DECREMENT COUNTER
               bne       LC945               BRANCH IF NOT DONE
               ldb       #$03                3 BYTES IN EXTENSION
               jsr       >LA59A              MOVE B BYTES FROM (X) TO (U)
               jsr       >L8748              EVALUATE A STRING EXPRESSION
               leau      ,X                  POINT U TO START OF STRING

               bra       LC96A               Jump around patch
FIXBUG         lbne      LCB97               Continue old code
               puls      u,x,d               -DITTO-
               cmpu      $94A                Is buffer end > pointer?
               lblo      LCBB4               No, continue old code
               ldx       #$989               Yes, reset start of buffer
               lbra      LCBC0               Store start of random buffer
*	CMPB	#$02	* aHECK LENGTH OF STRING AND
*	BLO	LC96A	* BRANCH IF < 2
*	LDA	$01,U	= GET 2ND CHARACTER IN STRING AND
*	CMPA	#':	= CHECK FOR COLON
*	BNE	LC96A	BRANCH IF NO DRIVE NUMBER
*	LDA	,U	* GET 1ST CHARACTER
*	CMPA	#'0	* IN STRING AND
*	BLO	LC96A	* CHECK TO SEE
*	CMPA	#'3	* IF IT IS IN
*	BHI	LC96A	* THE RANGE 0-3
*	BSR	LC99D	GET DRIVE NUMBER
LC96A          ldx       #DNAMBF             POINT X TO FILE NAME BUFFER
               incb                          COMPENSATE FOR DECB BELOW
LC96E          decb                          DECREMENT STRING LENGTH
               bne       LC97D               BRANCH IF MORE CHARACTERS IN STRING
               leas      $01,S               CLEAN UP STACK - REMOVE DRIVE FLAG
LC973          cmpx      #DNAMBF             POINTER STILL AT START OF BUFFER?
               bne       LC9DF               RETURN IF NOT
LC978          ldb       #2*31               'BAD FILENAME' ERROR IF NULL FILENAME
               jmp       >LAC46              ERROR HANDLER
LC97D          lda       ,U+                 GET A CHARACTER FROM STRING
               cmpa      #'.                 LOOK FOR PERIOD?
               beq       LC9B0               YES
               cmpa      #'/                 SLASH?
               beq       LC9B0               YES
               cmpa      #':                 COLON?
               beq       LC994               YES
               cmpx      #DEXTBF             COMPARE POINTER TO END OF FILENAME BUFFER
               beq       LC978               'BAD FILENAME' ERROR - FILENAME TOO LONG
               bsr       LC9D0               PUT A CHARACTER IN FILENAME
               bra       LC96E               GET ANOTHER CHARACTER FROM STRING
LC994          bsr       LC973               'BAD FILENAME' ERROR IF NO FILENAME YET
               bsr       LC99D               GET DRIVE NUMBER
               tstb                          * CHECK LENGTH OF STRING
               bne       LC978               * ''BAD FILENAME' ERROR IF MORE CHARACTERS LEFT
LC99B          puls      A,PC                REMOVE DRIVES FLAG FROM STACK AND RETURN
* GRAB DRIVE NUMBER
LC99D          com       $02,S               TOGGLE DRIVE FLAG
               beq       LC978               'BAD FILENAME' ERROR IF DRIVE NUMBER DEFINED TWICE

**       clr     <DCDRV
               nop       
               nop       
               lbsr      NUMCAL
               lda       <DCDRV
               cmpa      >MAXDRV
               lbhi      LA61F

*	LDA	,U++	ASCII VALUE OF DRIVE NUMBER TO ACCA
*	SUBB	#$02	DECREMENT STRING LENGTH BY 2 FOR DRIVE (:X)
*	SUBA	#'0	SUBTRACT ASCII BIAS
*	BLO	LC978	DRIVE NUMBER TOO LOW - 'BAD FILENAME' ERROR
*	CMPA	#$03	MAX OF 4 DRIVES
*	BHI	LC978	DRIVE NUMBER TOO HIGH - 'BAD FILENAME' ERROR
*	STA	DCDRV	STORE IN DSKCON DRIVE NUMBER
               rts       
* GRAB EXTENSION
LC9B0          bsr       LC973               'BAD FILENAME' ERROR IF NO FILENAME YET
               ldx       #DFLTYP             POINT X TO END OF EXTENSION BUFFER
               lda       #SPACE              BLANK
LC9B7          sta       ,-X                 *
               cmpx      #DEXTBF             * FILL EXTENSION WITH
               bne       LC9B7               * BLANKS (DEFAULT)
LC9BE          decb                          DECREMENT STRING COUNTER
               beq       LC99B               RETURN IF ZERO
               lda       ,U+                 GET A CHARACTER FROM STRING
               cmpa      #':                 *CHECK FOR DRIVE SEPARATOR
               beq       LC994               *
               cmpx      #DFLTYP             =CHECK FOR END OF ESTENSION RAM BUFFER &
               beq       LC978               = 'BAD FILENAME' ERROR IF EXTENSION TOO LONG
               bsr       LC9D0               PUT A CHARACTER IN EXTENSION BUFFER
               bra       LC9BE               GET ANOTHER EXTENSION CHARACTER
* INSERT CHARACTER INTO FILENAME OR EXTENSION
LC9D0          sta       ,X+                 STORE CHARACTER IN FILENAME BUFFER
               beq       LC978               'BAD FILENAME' ERROR; ZEROES ARE ILLEGAL
               cmpa      #'.                 PERIOD?
               beq       LC978               'BAD FILENAME' ERROR IF PERIOD
               cmpa      #'/                 SLASH?
               beq       LC978               'BAD FILENAME' ERROR IF SLASH
               inca                          CHECK FOR $FF
               beq       LC978               'BAD FILENAME' ERROR IF $FF
LC9DF          rts       
* SAVE COMMAND
SAVE           cmpa      #'M                 *
               lbeq      LCF68               *BRANCH IF SAVEM
               bsr       LCA33               GO GET FILENAME, ETC. FROM BASIC
               ldx       ZERO                ZERO OUT X REG
               stx       DFLTYP              SET FILE TYPE AND ASCII FLAG TO ZERO
               jsr       GETCCH              GET CURRENT INPUT CHARACTER FROM BASIC
               beq       LCA12               BRANCH IF END OF LINE
               jsr       SYNCOMMA            SYNTAX CHECK FOR COMMA
               ldb       #'A                 *ASCII FILE?
               jsr       >LB26F              *SYNTAX CHECK ON CONTENTS OF ACCB
               bne       LC9DF               RETURN IF NO MORE CHARACTERS ON LINE
               com       DASCFL              SET CRUNCHED/ASCII FLAG TO ASCII
               bsr       LCA04               OPEN A SEQUENTIAL FILE FOR OUTPUT
               clra                          SET	ZERO FLAG - CAUSE ENTIRE FILE TO BE LISTED
               jmp       LIST                'LIST' THE FILE TO CONSOLE OUT
* OPEN A SEQUENTIAL FILE FOR INPUT/OUTPUT - USE THE SYSTEM
* FCB LOCATED AT THE TOP OF FCBS
LCA04          lda       #'O                 OUTPUT FILE TYPE
LCA06          fcb       $8C                 SKIP TWO BYTES (THROWN AWAY CMPX INSTRUCTION)
LCA07          lda       #'I                 INPUT FILE TYPE
               ldb       FCBACT              GET NUMBER OF RESERVED FILES CURRENTLY RESERVED
               incb                          ADD ONE - USE ONE ABOVE HIGHEST RESERVED FCB
               stb       DEVNUM              SAVE IT IN DEVICE NUMBER
               jmp       >LC48D              OPEN A FILE & INITIALIZE FCB
* SAVE A CRUNCHED FILE - A PREAMBLE OF THREE BYTES WILL PRECEED CRUNCHED
* FILES: BYTE 1 = $FF, 2,3 = LENGTH OF BASIC PROGRAM
LCA12          bsr       LCA04               OPEN A SEQUENTIAL FILE FOR OUTPUT
               lda       #$FF                BASIC FILE FLAG
               jsr       >LCC24              CONSOLE OUT
               ldd       VARTAB              LOAD ACCD WITH START OF VARIABLES
               subd      TXTTAB              SUBTRACT START OF BASIC
               jsr       >LCC24              CONSOLE OUT FILE LENGTH MS BYTE
               tfr       B,A                 PULL LS BYTE INTO ACCA
               jsr       >LCC24              CONSOLE OUT FILE LENGTH LS BYTE
               ldx       TXTTAB              POINT X TO START OF BASIC
LCA27          lda       ,X+                 GET BYTE FROM BASIC
               jsr       >LCC24              SEND TO CONSOLE OUT
               cmpx      VARTAB              COMPARE TO END OF BASIC
               bne       LCA27               KEEP GOING IF NOT AT END
               jmp       >LA42D              CLOSE FILE
LCA33          ldx       #BASEXT             POINT TO 'BAS' EXTENSION (DEFAULT)
               jmp       >LC938              GET FILENAME.EXT FROM BASIC
* MERGE COMMAND
MERGE          clra                          RUN FLAG (0 = DON'T RUN)
               ldb       #$FF                MERGE FLAG ($FF = MERGE)
               bra       LCA50               GO LOAD THE FILE
* RUN RAM VECTOR
DVEC18         cmpa      #'"                 CHECK FOR FILENAME DELIMITER (DOUBLE QUOTE)
               lbne      XVEC18              NONE - JUMP TO EXBAS RUN RAM HOOK
               lda       #$02                RUN FLAG - DON'T CLOSE ALL FILES BEFORE RUN
               bra       LCA4F               LOAD THE FILE
* LOAD COMMAND
LOAD           cmpa      #'M                 *
               lbeq      LCFC1               *BRANCH IF LOADM
               clra                          RUN	FLAG = ZERO (DON'T RUN)
LCA4F          clrb                          CLEAR	MERGE FLAG
LCA50          sta       DRUNFL              RUN FLAG (0 = DON'T RUN, 2 = RUN)
               stb       DMRGFL              MERGE FLAG (0 = NO MERGE, $FF = MERGE)
               bsr       LCA33               GO GET FILENAME, ETC. FROM BASIC
               jsr       GETCCH              GET CURRENT INPUT CHAR
               beq       LCA6C               BRANCH IF END OF LINE
               jsr       SYNCOMMA            SYNTAX CHECK FOR COMMA
               ldb       #'R                 *
               jsr       >LB26F              *IS NEXT CHAR 'R'? RUN AFTER LOAD
               jsr       >LA5C7              SYNTAX ERROR IF ANY MORE CHARS ON LINE
LCA67          lda       #$03                *SET FLAGS TO RUN AND CLOSE ALL FILES
               sta       DRUNFL              *BEFORE THE FILE IS RUN
LCA6C          bsr       LCA07               GRAB FCB FOR INPUT FILE
               lda       DASCFL              *CHECK ASCII FLAG AND BRANCH
               beq       LCA7E               *IF CRUNCHED BASIC FILE
               tst       DMRGFL              IS THIS A MERGE?
               bne       LCA7B               BRANCH IF MERGE
               jsr       >LAD19              DO A 'NEW' - ERASE VARIABLES, RESET VARIABLES
LCA7B          jmp       >LAC7C              GO TO BASIC'S MAIN LOOP, IT WILL LOAD PROGRAM
* LOAD IN A CRUNCHED BASIC FILE
LCA7E          lda       DFLTYP              *CHECK FILE TYPE (MUST BE BASIC:0) & CHECK
               ora       DMRGFL              *MERGE FLAG (MUST BE NO MERGE: 0)
               lbne      LA616               'BAD FILE MODE' ERROR IF MERGE OR NON-BASIC
               jsr       >LAD19              DO A 'NEW' - RESET POINTERS, ERASE VARIABLES
               com       DLODFL              * SET THE LOAD FLAG TO $FF - THIS WILL CAUSE A NEW TO
* * OCCUR IF AN ERROR OCCURS WHILE THE PROGRAM IS BEING LOADED
               jsr       >LCDBC              GET CHAR FROM BUFFER - SHOULD BE $FF
               jsr       >LCDBC              GET ANOTHER - MS BYTE OF LENGTH
               pshs      A                   SAVE MS BYTE ON STACK
               jsr       >LCDBC              LS BYTE OF LENGTH OF PROGRAM
               tfr       A,B                 PUT LS BYTE INTO ACCB
               puls      A                   NOW ACCD CONTAINS LENGTH OF PROGRAM
               addd      TXTTAB              ADD BEGINNING OF BASIC
               jsr       >LAC37              SEE OF ENOUGH ROOM IN RAM FOR THIS FILE
               ldx       TXTTAB              GET START OF BASIC
LCAA4          jsr       >LC5C4              READ A CHAR FROM CONSOLE IN
               ldb       CINBFL              BUFFER EMPTY?
               bne       LCAAF               BRANCH IF SO
               sta       ,X+                 STORE CHAR
               bra       LCAA4               GET ANOTHER CHARACTER
LCAAF          clr       DLODFL              CLEAR LOAD FLAG - LOAD WAS ERROR FREE
               stx       VARTAB              SAVE NEW START OF VARIABLES
* MAKE SURE LAST THREE BYTES LOADED WERE ZERO
               ldb       #$03                CHECK THREE BYTES
LCAB6          lda       ,-X                 CHECK A BYTE
               bne       LCABD               BRANCH IF NON-ZERO
               decb                          DECREMENT COUNTER
               bne       LCAB6               KEEP CHECKING IF NOT DONE
LCABD          ldx       VARTAB              GET START OF VARIABLES
LCABF          stx       VARTAB              SAVE START OF VARIABLES
               clr       ,X+                 CLEAR A BYTE
               decb                          DECREMRNT COUNTER
               bpl       LCABF               KEEP CLEARING BYTES IF NOT DONE
LCAC6          jsr       >LA42D              CLOSE SELECTED FILE
               jsr       >LAD21              DO PART OF NEW - ERASE VARIABLES, RESET INPUT PTR
               jsr       XVEC18              INITIALIZE EXBAS GRAPHICS VARIABLES
               jsr       >LACEF              RELOCATE ALL THE BASIC NEXT LINE POINTERS
               asr       DRUNFL              CHECK LSB OF RUN FLAG
               blo       LCADA               BRANCH IF DON'T CLOSE ALL FILES
               jsr       >LA426              CLOSE ALL FILES
LCADA          asr       DRUNFL              TEST BIT 1 OF RUN FLAG
               lbcs      LAD9E               BRANCH TO COMM INTERPRETATION LOOP IF BIT 1 SET
               jmp       >LAC73              RETURN TO DIRECT MODE
DVEC13         tst       DEVNUM              * CHECK DEVICE NUMBER AND
               bgt       LCAC6               * TRY TO RUN FILE IF IT IS A DISK FILE
               rts       
* CLOSE ALL FILE BUFFERS RAM VECTOR
DVEC7          ldb       FCBACT              GET THE NUMBER OF RESERVED FILE BUFFERS
               incb                          ADD ONE
LCAED          pshs      B                   SAVE IT
               stb       DEVNUM              STORE IT IN DEVICE NUMBER
               bsr       LCB01               CLOSE FILE
               puls      B                   GET BACK NUMBER OF FILE BUFFERS
               decb                          DECREMENT FILE BUFFER COUNTER
               bne       LCAED               BRANCH IF ALL FILES NOT CLOSED
LCAF8          rts       
* CLOSE FILE RAM HOOK
DVEC8          tst       DEVNUM              * CHECK DEVICE NUMBER AND RETURN
               lble      XVEC8               * IF NOT A DISK FILE
               leas      $02,S               PURGE RETURN ADDRESS OFF OF THE STACK
LCB01          jsr       >LC744              POINT X TO CORRECT FCB
               clr       DEVNUM              SET DEVICE NUMBER TO SCREEN
LCB06          stx       FCBTMP              SAVE FILE BUFFER POINTER
               lda       FCBTYP,X            GET THE TYPE OF THIS FILE
               beq       LCAF8               RETURN IF FILE NOT OPEN
               pshs      A                   SAVE FILE TYPE
               clr       FCBTYP,X            CLOSE THE FILE - ZERO OUT THE FILE TYPE
               ldb       FCBDRV,X            * GET DRIVE NUMBER AND
               stb       DCDRV               * SAVE IT IN DSKCON VARIABLE
               cmpa      #OUTFIL             = CHECK FOR OUTPUT TYPE AND
               bne       LCB31               = BRANCH IF NOT OUTPUT TYPE FILE
* CLOSE A SEQUENTIAL OUTPUT FILE
               ldb       FCBLFT,X            GET THE NUMBER OF CHARACTERS IN BUFFER
               lda       #$80                * SET THE PRE-SAVED BIT TO INDICATE THAT THE DATA
* * HAS ALREADY BEEN SAVED ON DISK
               ora       FCBCPT,X            'OR' IN THE FULL SECTOR FLAG
               std       FCBLST,X            SAVE THE NUMBER OF BYTES USED IN THE LAST SECTOR
               inc       FCBSEC,X            INCREMENT THE SECTOR NUMBER
               ldb       FCBCGR,X            GET THE CURRENT GRANULE NUMBER
               jsr       >LC755              POINT X TO FILE ALLOCATION TABLE
               sta       FAT1,X              SET FAT DATA NOT VALID FLAG (ACCA <> 0)
               abx                           ADD GRANULE OFFSET TO FAT POINTER
               inc       FATCON,X            * INCREMENT GRANULE DATA (ADD ONE SECTOR TO LAST
* * GRANULE) SKIP PAST THE SIX FAT CONTROL BYTES
LCB2E          jmp       >LCBC3              UPDATE FAT AND DIRECTORY
LCB31          cmpa      #RANFIL             RANDOM FILE?
               bne       LCB2E               NO - UPDATE FAT AND DIRECTORY IF SEQUENTIAL INPUT FILE
* CLOSE A RANDOM FILE
               ldd       FCBRLN,X            GET RECORD LENGTH
               ldx       FCBBUF,X            POINT X TO RANDOM FILE BUFFER
               leay      D,X                 POINT Y TO END OF RANDOM FILE BUFFER
               pshs      Y,X,B,A             SAVE POINTERS ON STACK
               leay      ,S                  POINT Y CURRENT STACK POINTER
               ldu       VARTAB              GET START OF VARIABLES
LCB41          cmpu      ARYTAB              COMPARE TO START OF ARRAYS
               beq       LCB54               BRANCH IF ALL VARIABLES CHECKED
               lda       $01,U               GET 2ND BYTE OF VARIABLE NAME
               leau      $02,U               MOVE POINTER TO START OF DESCRIPTOR
               bpl       LCB4E               BRANCH IF VARIABLE - NUMERIC
               bsr       LCB76               ADJUST STRING VARIABLE IF IN RANDOM FILE BUFFER
LCB4E          leau      $05,U               MOVE POINTER TO NEXT VARIABLE
               bra       LCB41               PROCESS ANOTHER VARIABLE
LCB52          puls      U                   GET ADDRESS OF NEXT ARRAY TO U
LCB54          cmpu      ARYEND              COMPARE TO END OF ARRAYS
               beq       LCB93               BRANCH IF END OF ARRAYS
               tfr       U,D                 * SAVE ARRAY START IN ACCD, ADD OFFSET
               addd      $02,U               * TO NEXT ARRAY AND SAVE ADDRESS OF
               pshs      B,A                 * NEXT ARRAY ON THE STACK
               lda       $01,U               GET 2ND LETTER OF VARIABLE NAME
               bpl       LCB52               BRANCH IF NUMERIC
               ldb       $04,U               GET THE NUMBER OF DIMENSIONS
               aslb                          X2:2 BYTES PER DIMENSION
               addb      #$05                5 BYTES CONSTANT PER ARRAY DESCRIPTOR
               clra                          CLEAR	MSB OF OFFSET - (ONLY 125 DIMENSIONS ALLOWED)
               leau      D,U                 POINT U TO START OF THIS ARRAY'S VARIABLES
LCB6B          cmpu      ,S                  AT END OF THIS ARRAY?
               beq       LCB52               YES
               bsr       LCB76               ADJUST STRING VARIABLE IF IN RANDOM FILE BUFFER
               leau      $05,U               MOVE POINTER TO NEXT DESCRIPTOR
               bra       LCB6B               CHECK NEXT VARIABLE
*
* CHECK TO SEE IF A STRING IS LOCATED IN THE RANDOM FILE BUFFER AREA. IF IT IS
* THE RANDOM FILE BUFFER IN QUESTION, IT WILL BE DELETED. IF IT IS HIGHER IN THE RANDOM
* FILE BUFFER SPACE THAN THE BUFFER IN QUESTION, THE LENGTH OF THE CURRENT
* BUFFER WILL BE SUBTRACTED FROM THE ADDRESS OF THE STRING BECAUSE THE CURRENT
* BUFFER IS BEING DELETED (CLOSED).
LCB76          ldx       $02,U               POINT X TO START OF STRING
               cmpx      RNBFAD              COMPARE TO START OF FREE RANDOM FILE BUFFER AREA
               bhs       LCB8B               RETURN IF > START OF FREE RANDOM FILE BUFFER AREA
               cmpx      $02,Y               COMPARE TO START OF THIS FILE'S RANDOM BUFFER
               blo       LCB8B               RETURN IF < START OF THIS FILE'S RANDOM BUFFER
               cmpx      $04,Y               COMPARE TO END OF THIS FILE'S RANDOM BUFFER
               blo       LCB8C               RETURN IF < END OF THIS FILE'S RANDOM BUFFER
               tfr       X,D                 SAVE POINTER IN ACCD
               subd      ,Y                  SUBTRACT RECORD LENGTH FROM START OF STRING ADDRESS
               std       $02,U               SAVE NEW START OF STRING ADDRESS
LCB8B          rts       
LCB8C          clr       ,U                  CLEAR THE LENGTH OF THE STRING
               clr       $02,U               * CLEAR THE ADDRESS
               clr       $03,U               * OF THE STRING
               rts       
* REMOVE RESERVED SPACE IN RANDOM FILE BUFFER FOR A 'CLOSED' RANDOM FILE
* ADJUST THE START OF RANDOM FILE BUFFER POINTER IN ALL RANDOM FCBS
LCB93          ldb       FCBACT              GET THE NUMBER OF ACTIVE FILES
               incb                          ADD ONE
LCB97          pshs      B                   SAVE FILES COUNT ON THE STACK
               jsr       >LC749              POINT X TO FCB
               lda       FCBTYP,X            GET FILE TYPE
               cmpa      #RANFIL             IS IT A RANDOM FILE?
               bne       LCBAD               BRANCH IF NOT
               ldd       FCBBUF,X            GET START OF THIS FILE'S RANDOM FILE BUFFER
               cmpd      $04,Y               * COMPARE TO END OF RANDOM FILE BUFFER AREA AND
               blo       LCBAD               * BRANCH IF < END OF RANDOM FILE BUFFER AREA
               subd      ,Y                  = SUBTRACT RECORD LENGTH OF SELECTED FILE
               std       FCBBUF,X            = SAVE NEW START OF RANDOM FILE BUFFER
LCBAD          puls      B                   GET THE FILES COUNTER
               decb                          DECREMENT FILES COUNTER
               lbra      FIXBUG
               nop       
*	BNE	LCB97	BRANCH IF ALL FILES NOT DONE
*	PULS	A,B,X,U	* U = END OF RANDOM FILE BUFFER, X = START OF RANDOM
* FILE BUFFER, ACCD = RECORD LENGTH
** THIS WOULD PROBABLY BE THE MOST CONVENIENT PLACE TO FIX THE BUG WHICH
** CAUSES THE SYSTEM TO HANG IF AN ERROR IS ENCOUNTERED DURING 'COPY'
* CMPU FCBADR * IS THE END OF THIS FCB'S BUFFER ABOVE THE END
* * OF THE START OF THE FCB AREA
* BLO LCBB4 NO - FREE UP THE SPACE USED BY THIS FILE IN RANDOM BUFFER
* LDX #DFLBUF YES - DOING A 'COPY'; RESET START OF RANDOM BUFFER
* BRA LCBC0
* RANDOM FILE BUFFER AREA
* REMOVE RESERVED SPACE FOR CLOSED FILE FROM RANDOM FILE BUFFER SPACE
LCBB4          cmpu      RNBFAD              AT THE BOTTOM OF FREE RANDOM BUFFER AREA?
               beq       LCBC0               BRANCH IF THERE
               lda       ,U+                 = GRAB A SOURCE BYTE AND
               sta       ,X+                 = MOVE IT TO DESTINATION
               bra       LCBB4               KEEP MOVING BYTES
LCBC0          stx       RNBFAD              SAVE NEW START OF FREE RANDOM BUFFER AREA
LCBC3          jsr       >LC755              POINT X TO PROPER FILE ALLOCATION TABLE
               dec       FAT0,X              REMOVE ONE ACTIVE FILE
               tst       FAT1,X              NEW DATA IN FAT RAM IMAGE?
               beq       LCBCF               NO
               jsr       >LC71E              WRITE OUT FILE ALLOCATION TABLE TO DISK
LCBCF          ldx       FCBTMP              GET FILE BUFFER POINTER
               puls      A                   GET FILE TYPE
               cmpa      #OUTFIL             IS IT A SEQUENTIAL OUTPUT FILE?
               beq       LCBDF               YES
               cmpa      #RANFIL             IS IT A RANDOM FILE?
               bne       LCB8B               RETURN IF NOT A RANDOM FILE (SEQUENTIAL INPUT)
               lda       FCBFLG,X            * TEST THE GET/PUT FLAG AND
               beq       LCBE9               * BRANCH IF 'GET'
* WRITE CONTENTS OF FILE BUFFER TO DISK
LCBDF          jsr       >LC763              GET PROPER TRACK & SECTOR NUMBERS
               leau      FCBCON,X            POINT U TO START OF FCB DATA
               stu       DCBPT               SET UP FILE BUFFER POINTER FOR DSKCON
               bsr       LCC15               GO WRITE A SECTOR
LCBE9          lda       FCBLST,X            CHECK THE PRE-SAVED FLAG
               bpl       LCB8B               RETURN IF RECORD HAS ALREADY BEEN SAVED ON DISK
               ldb       FCBDIR,X            GET DIRECTORY NUMBER OF THIS FILE
               andb      #$07                8 ENTRIES PER SECTOR
               lda       #DIRLEN             DIRLEN BYTES PER DIRECTORY ENTRY
               mul                           GET SECTOR OFFSET FOR THIS ENTRY
               ldu       #DBUF0              * GET READ/WRITE BUFFER 0 AND
               stu       DCBPT               * SAVE IT IN DSKCON REGISTER
               leay      D,U                 Y POINTS TO CORRECT DIRECTORY ENTRY
               ldb       FCBDIR,X            GET DIRECTORY ENTRY NUMBER
               lsrb                          *
               lsrb                          *
               lsrb                          * DIVIDE BY 8; EIGHT DIRECTORY ENTRIES PER SECTOR
               addb      #$03                ADD BIAS; FIRST 3 SECTORS NOT DIRECTORY
               stb       DSEC                STORE SECTOR NUMBER
               ldd       #$1102              DIRECTORY TRACK - READ OP CODE
               sta       DCTRK               STORE TRACK NUMBER
               bsr       LCC17               GO READ DIRECTORY
               ldd       FCBLST,X            GET NUMBER OF BYTES IN THE LAST SECTOR
               anda      #$7F                MASK OFF THE PRE-SAVED FLAG
               std       DIRLST,Y            SAVE NUMBER OF BYTES IN LAST SECTOR OF FILE IN DIRECTORY
LCC15          ldb       #$03                WRITE OP CODE
LCC17          stb       DCOPC               SAVE DSKCON OP CODE VARIABLE
               jmp       >LD6F2              GO READ/WRITE SECTOR
* CONSOLE OUT RAM HOOK
DVEC3          lbra      ATARI
               fcb       $12,$12,$12
*DVEC3	TST	DEVNUM	CHECK DEVICE NUMBER
*	LBLE	XVEC3	BRANCH TO EX BASIC IF NOT A DISK FILE
LCC22          leas      $02,S               POP RETURN OFF STACK
* SEND A CHARACTER IN ACCA TO A DISK FILE. A CARRIAGE RETURN WILL RESET THE
* PRINT POSITION AND CONTROL CODES WILL NOT INCREMENT THE PRINT POSITION.
LCC24          pshs      X,B,A               SAVE REGISTERS
               ldx       #FCBV1-2            POINT X TO TABLE OF FILE NUMBER VECTORS
               ldb       DEVNUM              GET CURRENT FILE NUMBER
               aslb                          2 BYTES PER FCB ADDRESS
               ldx       B,X                 POINT X TO PROPER FCB
               ldb       ,X                  GET FILE TYPE (FCBTYP,X)
               cmpb      #INPFIL             IS IT AN INPUT FILE?
               beq       LCC6A               RETURN IF SO
               cmpa      #CR                 CARRIAGE RETURN (ENTER)
               bne       LCC3A               NO
               clr       FCBPOS,X            CLEAR PRINT POSITION IF CARRIAGE RETURN
LCC3A          cmpa      #SPACE              *
               blo       LCC40               *BRANCH IF CONTROL CHAR
               inc       FCBPOS,X            INCREMENT PRINT POSITION
LCC40          cmpb      #RANFIL             IS IT RANDOM FILE?
               bne       LCC5E               BRANCH IF NOT RANDOM
* PUT A BYTE INTO A RANDOM FILE
               ldd       FCBPUT,X            GET 'PUT' BYTE COUNTER
               addd      #$0001              ADD ONE
               cmpd      FCBRLN,X            COMPARE TO RECORD LENGTH
               lbhi      LCDCB               'FR' ERROR IF 'PUT' BYTE COUNTER > RECORD LENGTH
               std       FCBPUT,X            SAVE NEW 'PUT' BYTE COUNTER
               ldx       FCBBUF,X            POINT TO RANDOM FILE BUFFER POINTER
               leax      D,X                 POINT TO ONE PAST END OF CURRENT RECORD DATA
               puls      A                   PULL DATA FROM STACK
               sta       -1,X                STORE IN DATA BUFFER
               puls      B,X,PC              RESTORE REGISTERS AND RETURN
* WRITE A BYTE TO SEQUENTIAL OUTPUT FILE
LCC5E          inc       FCBLFT,X            INCREMENT CHARACTER COUNT
               ldb       FCBLFT,X            * GET CHARACTER COUNT AND BRANCH
               beq       LCC6C               * IF THE BUFFER IS FULL
               abx                           ADD CHARACTER COUNT TO FCB ADDRESS
               sta       FCBCON-1,X          STORE NEW CHARACTER (SKIP PAST 25 CONTROL BYTES AT FCB START)
LCC6A          puls      A,B,X,PC
* WRITE OUT A FULL BUFFER AND RESET BUFFER
LCC6C          pshs      U,Y                 SAVE REGISTERS
               sta       SECLEN+FCBCON-1,X   STORE LAST CHARACTER IN BUFFER
               ldb       FCBDRV,X            * GET DRIVE NUMBER AND SAVE
               stb       DCDRV               * IT IN DSKCON CONTROL TABLE
               inc       FCBSEC,X            INCREMENT SECTOR NUMBER
               jsr       >LCBDF              WRITE THE FILE BUFFER TO DISK
               leay      ,X                  SAVE FCB POINTER IN Y
               ldb       FCBCGR,X            GET GRANULE NUMBER
               jsr       >LC755              POINT X TO PROPER ALLOCATION TABLE
               abx                           ADD THE GRANULE NUMBER TO FAT POINTER
               leau      FATCON,X            POINT U TO THE CORRECT GRANULE IN FAT - SKIP PAST THE SIX FAT CONTROL BYTES
               lda       FCBSEC,Y            GET CURRENT SECTOR FOR THIS GRANULE
               cmpa      #$09                MAX SECTOR NUMBER (9 SECTORS/GRANULE)
               blo       LCC99               BRANCH IF NOT AT END OF GRANULE
               dec       FCBSEC,Y            *DECREMENT SECTOR NUMBER AND INCREMENT ERROR FLAG IN
               inc       FCBCPT,Y            *CASE ERROR FOUND WHILE LOOKING FOR NEXT GRANULE —
* THE ERROR FLAG IS USED TO INDICATE THAT ANOTHER SECTOR
* MUST BE ADDED TO THE LENGTH OF FILE FOLLOWING ERROR PROCESSING.
               jsr       >LC7BF              GET NEXT FREE GRANULE
               clr       FCBSEC,Y            *CLEAR SECTOR NUMBER AND
               clr       FCBCPT,Y            *ERROR FLAG - DISK WAS NOT FULL
               sta       FCBCGR,Y            SAVE NEW GRANULE IN FCB
               fcb       $8C                 SKIP TWO BYTES — NO DATA STORED IN NEW SECTOR YET (THROWN AWAY CMPX INSTRUCTION)
LCC99          ora       #$C0                FORCE GRANULE NUMBER TO BE FINAL GRANULE IN FILE
               sta       ,U                  STORE IN MAP
               leax      ,Y                  POINT X TO FCB
               jsr       >LC685              INCREMENT RECORD NUMBER
               jsr       >LC5A9              UPDATE FILE ALLOCATION TABLE
               puls      Y,U                 RESTORE REGISTERS
               puls      A,B,X,PC            RESTORE REGISTERS AND RETURN
* DIR COMMAND
*DIR	JSR	>LD24F	SCAN DRIVE NUMBER FROM INPUT LINE
DIR            jsr       >DIRCHK
               jsr       >LC79D              GET FAT FOR THIS DRIVE
               jsr       >SETSIZ
*	JSR	>LB958	PRINT CARRIAGE RETURN TO CONSOLE OUT
               ldd       #$1102              * GET TRACK 17 AND
               sta       DCTRK               * READ OP CODE AND
               stb       DCOPC               * SAVE IN DSKCON VARIABLES
               ldb       #$03                START WITH SECTOR 3 (FIRST DIRECTORY SECTOR)
* READ A DIRECTORY SECTOR INTO THE I/O BUFFER
LCCBB          stb       DSEC                SAVE SECTOR NUMBER IN DSKCON VARIABLE
               ldx       #DBUF0              * USE I/O BUFFER 0 FOR DATA TRANSFER
               stx       DCBPT               * SAVE IN DSKCON VARIABLE
               jsr       >LD6F2              READ A SECTOR
* SEND DIRECTORY INFORMATION TO CONSOLE OUT
LCCC5          puls      U                   SAVE TOP OF STACK
               jsr       >LA549              GO DO A BREAK CHECK
               pshs      U                   RESTORE STACK
               lda       ,X                  TEST FILE NAME FIRST BYTE (DIRNAM,X)
               beq       LCD08               BRANCH IF KILLED
               coma                          FF = END OF DIRECTORY
*	BEQ	LCD17	RETURN IF END OF DIRECTORY
               beq       HMM
               pshs      X                   SAVE DIRECTORY POINTER ON STACK
               ldb       #$08                NUMBER CHARACTERS TO PRINT
               jsr       >LB9A2              SEND FILENAME TO CONSOLE OUT
               bsr       PCCFA
*	BSR	LCD1B	SEND BLANK TO CONSOLE OUT
               ldb       #$03                NUMBER CHARACTERS TO PRINT
               jsr       >LB9A2              SEND EXTENSION TO CONSOLE OUT
               bsr       LCD1B               Print a space
               ldx       ,s                  Get directory pointer
               ldb       $0D,x               Get first gran in dir entry
               bsr       LCD1E               Count granules, result to A
               tfr       A,B                 Put data in B
               cmpb      #10                 Is it a 2 digit number?
               bhs       a@                  Yes, do not add filler space
               bsr       LCD1B               Add filler space
a@             clra                          Zero top of D
               jsr       LBDCC               Print decimal D to screen
               jsr       >DCOUNT             Columnize directory
               bra       b@                  Continue old code
PCCFA          jsr       >LCEAF
LCCFD          jmp       [CHROUT]
LCD01          pshs      cc,dp,d,x,y,u
               lbra      PD2A0
b@             puls      X
LCD08          leax      <$20,x
               cmpx      #$700
               blo       LCCC5
               ldb       <$ED
               incb      
               cmpb      #19
HMM            jmp       >BOISY

*	BSR	LCD1B	SEND BLANK TO CONSOLE OUT
*	LDB	FCBTYP,X	GET FILE TYPE
*	CMPB	#10	* CHECK THE NUMBER OF DECIMAL DIGITS IN
*	BHS	LCCEB	* ACCB: IF THERE IS ONLY ONE DIGIT,
*	BSR	LCD1B	* SEND BLANK TO CONSOLE OUT
*LCCEB	CLRA	CLEAR	MS BYTE OF ACCO
*	JSR	>LBDCC	PRINT ACCD IN DECIMAL TO CONSOLE OUT
*	BSR	LCD1B	SEND BLANK TO CONSOLE OUT
*	LDX	,S	X NOW POINTS TO DIRECTORY ENTRY
*	LDA	#'A+1	ASCII BIAS
*	ADDA	DIRASC,X	ADD TO ASCII FLAG
*	BSR	LCD18	PRINT CHARACTER AND BLANK TO CONSOLE OUT
*	LDB	DIRGRN,X	GET FIRST GRANULE IN FILE
*	BSR	LCD1E	COUNT GRANULES
*	TFR	A,B	SAVE COUNT IN ACCB
*	CLRA	CLEAR	MS BYTE OF ACCD
*	JSR	>LBDCC	PRINT ACCO IN DECIMAL TO CONSOLE OUT
*	JSR	>LB958	SEND CARRIAGE RETURN TO CONSOLE OUT
*	PULS	X	PULL DIRECTORY POINTER OFF OF THE STACK
*LCD08	LEAX	DIRLEN,X	MOVE X TO NEXT DIRECTORY ENTRY
*	CMPX	#DBUF0+SECLEN	END OF I/O BUFFER?
*	BLO	LCCC5	BRANCH IF MORE DIRECTORY ENTRIES IN BUFFER
*	LDB	DSEC	GET CURRENT SECTOR
*	INCB		BUMP COUNT
*	CMPB	#SECMAX	SECMAX SECTORS IN DIRECTORY TRACK
*	BLS	LCCBB	GET NEXT SECTOR
*LCD17	RTS	FINISHED
LCD18          jsr       PUTCHR              SEND CHARACTER TO CONSOLE OUT
LCD1B          jmp       >LB9AC              SEND BLANK TO CONSOLE OUT
* ENTER WITH ACCB POINTING TO FIRST GRANULE IN A FILE; RETURN THE NUMBER OF
* GRANULES IN THE FILE IN ACCA, THE GRANULE DATA FOR THE LAST SECTOR IN ACCB
LCD1E          jsr       >LC755              POINT X TO FILE ALLOCATION BUFFER
               leau      FATCON,X            POINT U TO START OF GRANULE DATA
               clra                          RESET	GRANULE COUNTER
LCD24          inca                          INCREMENT GRANULE COUNTER
               cmpa      #GRANMX             CHECKED ALL 68 GRANULES?
               lbhi      LC653               YES - 'BAD FILE STRUCTURE' ERROR
               leax      ,U                  POINT U TO START OF GRANULE DATA
               abx                           ADD POINTER TO FIRST GRANULE
               ldb       ,X                  GET THIS GRANULE'S CONTROL BYTE
               cmpb      #$C0                IS THIS THE LAST GRANULE IN FILE?
               blo       LCD24               NO - KEEP GOING
               rts       
* INPUT RAM HOOK
DVEC10         tst       DEVNUM              * CHECK DEVICE NUMBER AND RETURN
               ble       LCD97               * IF NOT A DISK FILE
               ldx       #LB069              = CHANGE THE RETURN ADDRESS ON THE STACK TO RE-ENTER BASIC'S INPUT
               stx       ,S                  = ROUTINE AT A DIFFERENT PLACE THAN THE CALLING ROUTINE
               ldx       #LINBUF+1           POINT X TO THE LINE INPUT BUFFER
               ldb       #',                 =
               stb       CHARAC              =COMMA IS READ ITEM SEPARATOR (TEMPORARY STRING SEARCH FLAG)
               lda       VALTYP              * GET VARIABLE TYPE AND BRANCH IF
               bne       LCD4B               * IT IS A STRING
               ldb       #SPACE              SPACE = NUMERIC SEARCH DELIMITER
LCD4B          bsr       LCDBC               GET AN INPUT CHARACTER
               cmpa      #SPACE              SPACE?
               beq       LCD4B               YES - GET ANOTHER CHARACTER
               cmpa      #'"                 QUOTE?
               bne       LCD5F               NO
               cmpb      #',                 SEARCH CHARACTER = COMMA?
               bne       LCD5F               NO - NUMERIC SEARCH
               tfr       A,B                 * SAVE DOUBLE QUOTE AS
               stb       CHARAC              * THE SEARCH FLAG
               bra       LCD81               SAVE DOUBLE QUOTES AS FIRST ITEM IN BUFFER
LCD5F          cmpb      #'"                 *
               beq       LCD74               *BRANCH IF INPUTTING A STRING VARIABLE
               cmpa      #CR                 IS THE INPUT CHARACTER A CARRIAGE RETURN
               bne       LCD74               NO
               cmpx      #LINBUF+1           *IF AT THE START OF INPUTBUFFER, CHECK FOR A
               beq       LCDB0               *FOLLOWING LINE FEED AND EXIT ROUTINE
               lda       -1,X                =IF THE INPUT CHARACTER PRECEEDING THE CR WAS A LINE FEED,
               cmpa      #LF                 =THEN INSERT THE CR IN THE INPUT STRING, OTHERWISE
               bne       LCDB0               =CHECK FOR A FOLLOWING LINE FEED AND EXIT THE ROUTINE
               lda       #CR                 RESTORE CARRIAGE RETURN AS THE INPUT CHARACTER
LCD74          tsta                          *CHECK FOR A NULL (ZERO) INPUT CHARACTER AND
               beq       LCD8E               *IGNORE IT IF lT IS A NULL
               cmpa      CHARAC              =
               beq       LCD98               =CHECK TO SEE IF THE INPUT CHARACTER MATCHES
               pshs      B                   =EITHER ACCB OR CHARAC AND IF IT DOES, THEN
               cmpa      ,S+                 =BRANCH TO CHECK FOR ITEM SEPARATOR OR
               beq       LCD98               =TERMINATOR SEQUENCE AND EXIT ROUTINE
LCD81          sta       ,X+                 STORE NEW CHARACTER IN BUFFER
               cmpx      #LINBUF+LBUFMX      END OF INPUT BUFFER
               bne       LCD8E               NO
               bsr       LCDD0               GET A CHARACTER FROM CONSOLE IN
               bne       LCD92               EXIT ROUTINE IF BUFFER EMPTY
               bra       LCDAC               CHECK FOR CR OR CR/LF AND EXIT ROUTINE
LCD8E          bsr       LCDD0               GET A CHARACTER FROM CONSOLE IN
               beq       LCD5F               BRANCH IF BUFFER NOT EMPTY
LCD92          clr       ,X                  PUT A ZERO AT END OF BUFFER WHEN DONE
               ldx       #LINBUF             POINT (X) TO LINBUF - RESET POINTER
LCD97          rts       
* CHECK FOR ITEM SEPARATOR OR TERMINATOR AND EXIT THE INPUT ROUTINE
LCD98          cmpa      #'"                 QUOTE?
               beq       LCDA0               YES
               cmpa      #SPACE              SPACE?
               bne       LCD92               NO - EXIT ROUTINE
LCDA0          bsr       LCDD0               GET A CHARACTER FROM CONSOLE IN
               bne       LCD92               EXIT ROUTINE IF BUFFER EMPTY
               cmpa      #SPACE              SPACE?
               beq       LCDA0               YES - GET ANOTHER CHARACTER
               cmpa      #',                 COMMA (ITEM SEPARATOR)?
               beq       LCD92               YES - EXIT ROUTINE
LCDAC          cmpa      #CR                 CARRIAGE RETURN?
               bne       LCDB8               NO
LCDB0          bsr       LCDD0               GET A CHARACTER FROM CONSOLE IN
               bne       LCD92               EXIT ROUTINE IF BUFFER EMPTY
               cmpa      #LF                 LINE FEED? TREAT CR,LF AS A CR
               beq       LCD92               YES - EXIT ROUTINE
LCDB8          bsr       LCDD6               BACK UP PTR INPUT POINTER ONE
               bra       LCD92               EXIT ROUTINE
LCDBC          bsr       LCDD0               GET A CHAR FROM INPUT BUFFER - RETURN IN ACCA
               beq       LCDD5               RETURN IF BUFFER NOT EMPTY
               jsr       >LC744              POINT X TO START OF FILE BUFFER
               ldb       FCBTYP,X            GET FILE TYPE
               cmpb      #RANFIL             IS IT RANDOM FILE TYPE?
               lbne      LC352               'INPUT PAST END OF FILE’ ERROR IF NOT RANDOM
LCDCB          ldb       #2*37               'WRITE/INPUT PAST END OF RECORD’ ERROR IF RANDOM
               jmp       >LAC46              JUMP TO THE ERROR HANDLER
LCDD0          jsr       >LA176              GET A CHAR FROM INPUT BUFFER
               tst       CINBFL              SET FLAGS ACCORDING TO CONSOLE INPUT FLAG
LCDD5          rts       
* MOVE THE INPUT POINTER BACK ONE (DISK FILE)
LCDD6          pshs      X,B                 SAVE REGISTERS ON STACK
               jsr       >LC744              POINT X TO PROPER FCB
               ldb       FCBTYP,X            GET FILE TYPE OF THIS FCB
               cmpb      #RANFIL             IS IT A RANDOM FILE?
               bne       LCDEC               BRANCH IF NOT A RANDOM FILE
               ldd       FCBGET,X            *GRAB THE RANDOM FILE 'GET' POINTER,
               subd      #$0001              *MOVE IT BACK ONE AND RESTORE IT
               std       FCBGET,X            *
               puls      B,X,PC              RESTORE REGISTERS AND RETURN
LCDEC          sta       FCBCDT,X            SAVE THE CHARACTER IN THE CACHE
               com       FCBCFL,X            SET THE CACHE FLAG TO $FF - DATA IN CACHE
               puls      B,X,PC              RESTORE REGISTERS AND RETURN
* CVN COMMAND
CVN            jsr       >LB654              GET LENGTH AND ADDRESS OF STRING
               cmpb      #$05                FIVE BYTES IN A FLOATING POINT NUMBER
               lbcs      LB44A               'FC' ERROR IF <> 5 BYTES
               clr       VALTYP              SET VARIABLE TYPE TO NUMERIC
               jmp       >LBC14              COPY A PACKED FP NUMBER FROM (X) TO FPA0
* MKN$ COMMAND
MKN            jsr       >LB143              'TM' ERROR IF VALTYP=STRING
               ldb       #$05                FIVE BYTES IN A FLOATING POINT NUMBER
               jsr       >LB50F              RESERVE FIVE BYTES IN STRING SPACE
               jsr       >LBC35              PACK FPA0 AND STORE IT IN STRING SPACE
               jmp       >LB69B              SAVE STRING DESCRIPTOR ON STRING STACK
* LOC COMMAND
LOC            bsr       LCE19               POINT X TO FILE BUFFER
               ldd       FCBREC,X            GET RECORD NUMBER (RANDOM FILE) OR SECTOR CTR (SEQUENTIAL)
LCE14          std       FPA0+2              *SAVE ACCD IN BOTTOM 2 BYTES OF FPA0 AND
               jmp       >L880E              *CONVERT TO FLOATING POINT NUMBER
* STRIP A DEVICE NUMBER FROM A BASIC STATEMENT, SET PRINT
* PARAMETERS ACCORDING TO IT - ERROR IF FILE NOT
* OPEN. RETURN WITH (X) POINTING TO THAT FILE'S FCB
LCE19          lda       DEVNUM              * GET CURRENT DEVICE NUMBER AND
               pshs      A                   * SAVE IT ON THE STACK
               jsr       >LB143              'TM' ERROR IF VALTYP=STRING
               jsr       >LA5AE              CHECK FOR VALID DEVICE NUMBER/SET PRINT PARAMETERS
               tst       DEVNUM              * CHECK DEVICE NUMBER
               lble      LB44A               * BRANCH IF NOT DISK FILE 'ILLEGAL FUNCTION CALL'
               jsr       >LC744              POINT (X) TO FILE BUFFER
               puls      A                   * GET OLD DEVICE NUMBER OFF OF THE STACK AND
               sta       DEVNUM              * SAVE IT AS DEVICE NUMBER
               tst       FCBTYP,X            IS FILE OPEN?
               lbeq      LA3FB               'FILE NOT OPEN' ERROR IF NOT OPEN
               rts       
* LOF
LOF            bsr       LCE19               POINT X TO FILE BUFFER
               lda       FCBDRV,X            * GET DRIVE NUMBER AND SAVE IT
               sta       DCDRV               * IN DSKCON VARIABLE
               ldb       FCBFGR,X            GET FIRST GRANULE OF FILE
               pshs      X                   SAVE FCB POINTER ON STACK
               jsr       >LCD1E              FIND TOTAL NUMBER OF GRANULES IN THIS FILE
               deca                          SUBTRACT THE LAST GRANULE IN THE FILE
               andb      #$3F                GET NUMBER OF SECTORS USED IN LAST GRANULE
               pshs      B                   SAVE NUMBER OF SECTORS IN LAST GRANULE ON STACK
               tfr       A,B                 * CONVERT ACCA TO POSITIVE
               clra                          *	2 BYTE VALUE IN ACCD
               jsr       >LC779              MULT NUMBER OF FULL GRANULES BY 9
               addb      ,S+                 ADD NUMBER SECTORS IN LAST TRACK
               adca      #$00                PROPAGATE CARRY TO MS BYTE OF ACCD
               puls      X                   GET FCB POINTER BACK
               pshs      A                   SAVE ACCA ON STACK
               lda       FCBTYP,X            * GET FILE TYPE OF THIS FCB AND
               cmpa      #RANFIL             * CHECK TO SEE IF IT'S A RANDOM FILE
               puls      A                   RESTORE ACCA
               bne       LCE14               IF NOT A RANDOM FILE, THEN THE TOTAL NUMBER OF SECTORS IN THE FILE
* IS THE LENGTH OF THE FILE
* CALCULATE LOF FOR A RANDOM FILE - THE LENGTH OF A RANDOM FILE IS THE
* NUMBER OF RECORDS IN THE FILE.
               pshs      X                   SAVE FCB POINTER ON STACK
               subd      ZERO                SUBTRACT ZERO FROM ACCD (NUMBER OF SECTORS)
               beq       LCE68               BRANCH IF ZERO SECTORS
               subd      #$0001              SUBTRACT ONE SECTOR - THE LAST SECTOR MAY NOT BE IOOZ USED
LCE68          bsr       LCE14               PUT ACCD INTO FPA0
               ldb       FP0EXP              GET EXPONENT OF FPA0
               beq       LCE72               BRANCH IF FPA0 = 0
               addb      #$08                * ADD 8 TO EXPONENT (MULTIPLY FPA0 BY
               stb       FP0EXP              * 256 BYTES/SECTOR) AND SAVE NEW EXPONENT
LCE72          jsr       >LBC5F              SAVE NUMBER OF BYTES IN FULL SECTORS IN FPA1
               ldx       ,S                  POINT X TO FCB
               ldd       FCBLST,X            GET NUMBER OF BYTES IN LAST SECTOR
               anda      #$7F                MASK OFF THE PRE-SAVED BYTE
               bsr       LCE14               PUT NUMBER BYTES IN LAST SECTOR INTO FPA0
               clr       RESSGN              FORCE SUM SIGN = POSITIVE
               lda       FP1EXP              * GET EXPONENTS OF FPA0 AND
               ldb       FP0EXP              * FPA1 PRIOR TO ADDITION
               jsr       >LB9C5              ADD NUMBER BYTES IN LAST SECTOR TO NUMBER OF	BYTES IN FULL SECTORS
               jsr       >LBC5F              SAVE TOTAL NUMBER OF BYTES IN FPA1
               puls      X                   POINT X TO FCB
               ldd       FCBRLN,X            * GET RECORD LENGTH
               bsr       LCE14               * PUT IT INTO FPA0
               clr       RESSGN              FORCE QUOTIENT SIGN = POSITIVE
               lda       FP1EXP              * GET EXPONENTS OF FPA0 AND
               ldb       FP0EXP              * FPA1 PRIOR TO DIVISION
               jsr       >LBB91              DIVIDE TOTAL NUMBER OF BYTES BY NUMBER OF BYTES IN A RECORD
               jmp       INT                 CONVERT FPA0 TO AN INTEGER
* FREE COMMAND
FREE           jsr       >LB143              * NUMBER TYPE CHECK
               jsr       >LB70E              *EVALUATE NUMERIC EXPRESSION AND RETURN VALUE IN ACCB
               bsr       HITEST              Test for > max drive
               stb       <DCDRV              Drive number
               jsr       >LC79D               Test file allocation table
               jsr       >BRIAN
               jmp       LB4F3
LCEAF          pshs      x
               ldx       6,s
               lda       $C,x
               anda      #1
               adda      #$2E
               puls      x,pc
HITEST         cmpb      >MAXDRV
               lbhi      LA61F
               rts       
               fcb       $12,$12             ,$12

*	CMPB	#$03	ONLY 4 LEGAL DRIVES
*	LBHI	LA61F	'DEVICE NUMBER' ERROR IF DRIVE NUMBER IS > 3
*	STB	DCDRV	SAVE IN DRIVE NUMBER
*	JSR	>LC79D	GET FILE ALLOCATION TABLE AND STORE IN BUFFER
*	JSR	>LC755	POINT X TO START OF FILE ALLOCATION TABLE BUFFER
*	LEAX	FATCON,X	MOVE TO FIRST GRANULE DATA BYTE
*	CLR	,-S	SPACE FOR FREE GRANULE COUNTER
*	LDB	#GRANMX	GET MAXIMUM NUMBER OF GRANULES
*LCEB6	LDA	,X+	GET GRANULE DATA
*	COMA		*FREE GRANULES $FF
*	BNE	LCEBD	*BRANCH IF NOT FREE
*	INC	,S	INCREMENT FREE GRANULE COUNTER
*LCEBD	DECB		DECREMENT GRANULE COUNTER
*	BNE	LCEB6	BRANCH IF NOT DONE
*	PULS	B	GET FREE GRANULE COUNTER TO ACCB
*	JMP	>LB4F3	LOAD ACCB INTO FPA0
* DRIVE COMMAND
DRIVE          jmp       >DRVCHK
               nop       
               nop       
*DRIVE	JSR	EVALEXPB	EVALUATE EXPR; RETURN VALUE IN ACCB
*	CMPB	#$03	MAX DRIVE NUMBER = 3
LCECA          lbhi      LA61F               'DEVICE #' ERROR IF DRIVE NUMBER > 3
               stb       DEFDRV              SAVE DEFAULT DRIVE NUMBER
               rts       
* EVALUATE EXPRESSION RAM VECTOR
DVEC15         lda       $04,S               = CHECK STACKED PRECEDENCE FLAG AND IF IT IS NOT AN END
               bne       LCEE9               = OF OPERATION, BRANCH TO EXTENDED BASIC'S EXPRESSION EVALUATION ROUTINE
               ldx       $05,S               *
               cmpx      #LAF9A              *
               bne       LCEE9               * CHECK TWO RETURN ADDRESSES BACK ON THE STACK
               ldx       $02,S               * TO SEE IF THE CALL TO EVALUATE EXPRESSION IS
               cmpx      #LB166              * COMING FROM THE 'LET' COMMAND - BRANCH OUT IF
               bne       LCEE9               * NOT COMING FROM 'LET'
               ldx       #LCEEC              = IF COMING FROM 'LET', REPLACE THE RETURN ADDR
               stx       $05,S               = WITH THE DISK BASIC 'LET' MODIFIER ADDRESS
LCEE9          jmp       XVEC15              EXTENDED BASIC EXPRESSION EVALUATION
* LET MODIFIER
LCEEC          puls      A                   PULL VARIABLE TYPE OFF OF THE STACK
               rora                          SET CARRY IF SIRING, CLEAR CARRY IF NUMERIC
               jsr       >LB148              DO A 'TM' CHECK
               lbeq      LBC33               IF NUMERIC VARIABLE, PACK FPA0 INTO VARDES
               ldx       FPA0+2              POINT X TO STRING DESCRIPTOR
               ldd       $02,X               GET ADDRESS OF SIRING
               cmpd      #DFLBUF             * COMPARE TO START OF RANDOM FILE BUFFERS
               blo       LCF07               * AND BRANCH IF LOWER
               subd      FCBADR              SUBTRACT OUT THE END OF RANDOM FILE BUFFERS
               lbcs      LAFB1               BRANCH IF STRING STORED IN RANDOM FILE BUFFER - MOVE IT INTO THE STRING SPACE
LCF07          jmp       >LAFA4              BRANCH BACK TO BASIC’S 'LET' COMMAND
*MODIFIER FOR EXBAS COMMAND INTERPRETATION HANDLER
DXCVEC         cmpa      #TOKEN_DLOAD        TOKEN FOR DLOAD?
               beq       LCF2A               YES
               cmpa      #TOKEN_PMODE        TOKEN for PMODE?
               lbne      L813C               NO
* DISK BASIC MODIFIER FOR PMODE - ALLOWS FOR THE RAM THE DOS USES
               jsr       GETNCH              GET NEXT CHARACTER FROM BASIC
               cmpa      #',                 CHECK FOR COMMA
               lbeq      L9650               BRANCH IF COMMA
               jsr       EVALEXPB            EVALUATE EXPRESSION; RETURN VALUE IN ACCB
               cmpb      #$04                CHECK FOR PMODE 4
               lbhi      LB44A               'FC' ERROR IF PMODE > 4
               lda       GRPRAM              NUMBER BLOCKS BEFORE GRAPHICS PAGES
               jmp       >L962E              JUMP TO EXBAS' PMODE COMMAND
* DISK BASIC DLOAD MODIFIER
LCF2A          jsr       >LA429              CLOSE FILES
               jsr       GETNCH              GET NEXT CHARACTER FROM BASIC
               jmp       >L8C1B              JUMP TO EXBAS' DLOAD
DXIVEC         cmpb      #(TOKEN_POS-$80)*2  MODIFIED TOKEN FOR POS
               lbne      L8168               IF NOT POS, GO TO EXBAS SECONDARY COMM HANDLER
               jsr       >LB262              SYNTAX CHECK FOR '(' AND EVALUATE EXPRESSION
               lda       DEVNUM              * GET DEVICE NUMBER AND
               pshs      A                   * SAVE IT ON STACK
               jsr       >LA5AE              EVALUATE DEVICE NUMBER
               jsr       >LA406              TEST DEVICE NUMBER
               tst       DEVNUM              * CHECK DEVICE NUMBER AND BRANCH
               ble       LCF5C               * IF NOT A DISK FILE
               jsr       >LC744              POINT X TO FCB
               ldb       FCBTYP,X            GET FILE TYPE
               cmpb      #RANFIL             DIRECT/RANDOM FILE?
               bne       LCF5C               BRANCH IF NOT A RANDOM FILE
               puls      A                   * RESTORE DEVICE NUMBER
               sta       DEVNUM              *
               ldd       FCBPUT,X            =GRAB THE 'PUT' DATA ITEM COUNTER AND CONVERT
               jmp       GIVABF              =IT TO A FLOATING POINT NUMBER
LCF5C          jsr       >LA35F              SET PRINT PARAMETERS
               puls      A                   * RESTORE DEVICE NUMBER
               sta       DEVNUM              *
               ldb       DEVPOS              =GET PRINT POSITION AND
               jmp       >LB4F3              =CONVERT IT TO FLOATING POINT NUMBER IN FPA0
* SAVEM COMMAND
LCF68          jsr       GETNCH              GET NEXT INPUT CHARACTER
               bsr       LCFBB               GET FILENAME, ETC.
               jsr       >L836C              EVALUATE EXPRESSION, PUT II (2 BYTES) ON STACK
               jsr       >L836C              DITTO
               cmpx      $02,S               COMPARE END ADDRESS TO START ADDRESS
               lbcs      LB44A               IF START > END, THEN 'ILLEGAL FUNCTION CALL'
               jsr       >L836C              EVAL EXPRESSION (TRANSFER ADDRESS), PUT ON STACK
               jsr       >LA5C7              SYNTAX ERROR IF ANY MORE CHARS ON THIS LINE
               ldd       #$0200              * FILE TYPE=2, ASCII FLAG = CRUNCHED (0)
               std       DFLTYP              *
               jsr       >LCA04              GET NEXT UNOPEN FILE AND INITIALIZE FCB
               clra                          *ZERO	FLAG - FIRST BYTE OF PREAMBLE
               bsr       LCFB5               *WRITE A BYTE TO BUFFER
               ldd       $02,S               GET END ADDRESS
               subd      $04,S               SUBTRACT THE START ADDRESS
               addd      #$0001              THE SAVED DATA BLOCK WILL INCLUDE BOTH THE FIRST AND LAST BYTES
               tfr       D,Y                 SAVE LENGTH IN Y
               bsr       LCFB3               WRITE FILE LENGTH TO BUFFER - FIRST ARGUMENT OF PREAMBLE
               ldd       $04,S               GET THE START ADDRESS
               bsr       LCFB3               WRITE OUT THE START ADDRESS - SECOND PREAMBLE ARGUMENT
               ldx       $04,S               GET START ADDRESS
LCF9B          lda       ,X+                 GRAB A BYTE
               jsr       >LCC24              WRITE IT OUT
               leay      -1,Y                DECREMENT BYTE COUNTER
               bne       LCF9B               BRANCH IF ALL BYTES NOT DONE
               lda       #$FF                FIRST BYTE OF POSTAMBLE
               bsr       LCFB5               WRITE IT OUT - EOF RECORD
               clra                          *	FIRST ARGUMENT OF POSTAMBLE IS
               clrb                          *	A DUMMY - ZERO VALUE
               bsr       LCFB3               WRITE OUT POSTAMBLE FIRST ARGUMENT
               puls      A,B,X,Y             GET CONTROL ADDRESSES FROM THE STACK
               bsr       LCFB3               WRITE OUT THE TRANSFER ADDRESS - 2ND ARGUMENT
               jmp       >LA42D              GO CLOSE ALL FILES
* WRITE ACCD TO THE BUFFER
LCFB3          bsr       LCFB5               WRITE ACCA TO BUFFER, THEN SWAP ACCA,ACCB
LCFB5          jsr       >LCC24              WRITE ACCA TO BUFFER
               exg       A,B                 SWAP ACCA,ACCB
               rts       
LCFBB          ldx       #BINEXT             POINT TO .BIN EXTENSION
               jmp       >LC938              GET FILENAME, ETC.
* LOADM COMMAND
LCFC1          jsr       GETNCH              GET NEXT INPUT CHARACTER
               bsr       LCFBB               GET FILENAME, ETC.
               jsr       >LCA07              OPEN NEXT AVAILABLE FILE FOR INPUT
               ldd       DFLTYP              GET FILE TYPE AND ASCII FLAG
               subd      #$0200              FOR LOADM FILE: TYPE=2, ASCII FLAG=0
               lbne      LA616               'BAD FILE MODE' ERROR
               ldx       ZERO                ZERO OUT X REG - DEFAULT VALUE OF OFFSET
               jsr       GETCCH              GET CURRENT CHARACTER FROM BASIC
               beq       LCFDE               BRANCH IF END OF LINE - NO OFFSET
               jsr       SYNCOMMA            SYNTAX CHECK FOR COMMA
               jsr       >LB73D              EVALUATE EXPRESSION
LCFDE          stx       VD3                 STORE OFFSET IN VD3
               jsr       >LA5C7              SYNTAX ERROR IF OTHER CHARACTERS ON LINE
* GET PREAMBLE/POSTAMBLE
LCFE3          jsr       >LCDBC              GET FIRST BYTE
               pshs      A                   SAVE IT ON THE STACK
               bsr       LD013               GET FIRST ARGUMENT
               tfr       D,Y                 SAVE IT IN Y
               bsr       LD013               GET THE SECOND ARGUMENT
               addd      VD3                 ADD IT TO THE OFFSET
               std       EXECJP              STORE IT IN THE JUMP ADDRESS OF THE EXEC COMMAND
               tfr       D,X                 SAVE IT IN X
               lda       ,S+                 GET THE FIRST BYTE OFF OF THE STACK
               lbne      LA42D               CLOSE FILE IF POSTAMBLE (EOF)
* GET RECORD BYTE(S)
LCFFA          jsr       >LC5C4              GET BYTE FROM BUFFER
               ldb       CINBFL              GET STATUS OF CONSOLE IN BUFFER
               beq       LD004               BRANCH IF BUFFER NOT EMPTY
               jmp       >LC352              'INPUT PAST END OF FILE' ERROR
LD004          sta       ,X                  STORE BYTE IN MEMORY
               cmpa      ,X+                 *TEST TO SEE IF IT STORED PROPERLY AND
               beq       LD00D               *BRANCH IF PROPER STORE (NOT IN ROM OR BAD RAM)
               jmp       >LD709              'I/O ERROR' IF BAD STORE
LD00D          leay      -1,Y                DECREMENT BYTE COUNT
               bne       LCFFA               GET NEXT BYTE IF NOT DONE
               bra       LCFE3               READ ANOTHER PRE/POST AMBLE
* READ TWO BYTES FROM BUFFER - RETURN THEM IN ACCD
LD013          bsr       LD015               READ A BYTE, SAVE IT IN ACCB
LD015          jsr       >LCDBC              GET A CHARACTER FROM INPUT BUFFER, RETURN IT IN ACCA
               exg       A,B                 SWAP ACCA,ACCB
               rts       
* RENAME COMMAND
RENAME                   
               jmp       >RENAME2
               nop       
*	LDX	CHARAD	* SAVE CURRENT INPUT POINTER
*	PSHS	X	* ON THE STACK
LD01F          bsr       LD056               GET FILENAME OF SOURCE FILE
               lda       DCDRV               * SAVE DRIVE NUMBER
               pshs      A                   * ON THE STACK
               bsr       LD051               SYNTAX CHECK FOR 'TO' AND GET NEW FILENAME
               puls      A                   GET SOURCE DRIVE NUMBER
               cmpa      DCDRV               COMPARE TO NEW FILE DRIVE NUMBER
               lbne      LB44A               'FC' ERROR IF FlIES ON DIFFERENT DRIVES
               bsr       LD059               VERIFY THAT NEW FILE DOES NOT ALREADY EXIST
               puls      X                   * RESTORE INPUT POINTER
               stx       CHARAD              *
               bsr       LD056               GET SOURCE FILENAME AGAIN
               jsr       >LC68C              SCAN DIRECTORY FOR SOURCE FILENAME
               jsr       >LC6E5              'NE' ERROR IF NOT FOUND
               bsr       LD051               SYNTAX CHECK FOR 'TO' AND GET NEW FILENAME
               ldx       #DNAMBF             POINT X TO FILENAME
               ldu       V974                POINT U TO DIRECTORY ENTRY OF SOURCE FILE
               ldb       #$0B                11 CHARACTERS IN FILENAME AND EXTENSION
               jsr       >LA59A              COPY NEW FILENAME TO SOURCE FILE DIRECTORY RAM IMAGE
               ldb       #$03                * GET WRITE OP CODE AND
               stb       DCOPC               * SAVE IN DSKCON VARIABLE
               jmp       >LD6F2              WRITE NEW DIRECTORY SECTOR
* DO A SYNTAX CHECK FOR 'TO’ AND STRIP A FILENAME FROM BASIC
LD051          ldb       #TOKEN_TO           'TO' TOKEN
               jsr       >LB26F              SYNTAX CHECK FOR 'TO'
LD056          jmp       >LC935              GET FILENAME FROM BASIC
LD059          jsr       >LC68C              SCAN DIRECTORY FOR FILENAME
LD05C          ldb       #33*2               'FILE ALREADY EXISTS' ERROR
               tst       V973                CHECK FOR A MATCH
               lbne      LAC46               'AE' ERROR IF FILE IN DIRECTORY
               rts       
* WRITE COMMAND
WRITE          lbeq      LB958               PRINT CARRIAGE RETURN TO CONSOLE OUT IF END OF LINE
               bsr       LD06F               GO WRITE AN ITEM LIST
               clr       DEVNUM              SET DEVICE NUMBER TO SCREEN
LD06E          rts       
LD06F          cmpa      #'#                 CHECK FOR DEVICE NUMBER FLAG
               bne       LD082               DEFAULT TO CURRENT DEVICE NUMBER IF NONE GIVEN
               jsr       >LA5A5              SET DEVICE NUMBER; CHECK VALIDITY
               jsr       >LA406              MAKE SURE SELECTED FILE IS AN OUTPUT FILE
               jsr       GETCCH              GET CURRENT INPUT CHARACTER
               lbeq      LB958               PRINT CR TO CONSOLE OUT IF END OF LINE
LD07F          jsr       SYNCOMMA            SYNTAX CHECK FOR COMMA
LD082          jsr       >LB156              EVALUATE EXPRESSION
               lda       VALTYP              GET VARIABLE TYPE
               bne       LD0A7               BRANCH IF STRING
               jsr       >LBDD9              CONVERT FP NUMBER TO ASCII STRING
               jsr       >LB516              PUT ON TEMPORARY STRING STACK
               jsr       >LB99F              PRINT STRING TO CONSOLE OUT
* PRINT ITEM SEPARATOR TO CONSOLE OUT
LD092          jsr       GETCCH              GET CURRENT CHARACTER
               lbeq      LB958               PUT CR TO CONSOLE OUT IF END OF LINE
               lda       #',                 COMMA: NON-CASSETTE SEPARATOR
               jsr       >LA35F              SET PRINT PARAMETERS
               tst       PRTDEV              * GET CONSOLE PRINT DEVICE AND
               beq       LD0A3               * BRANCH IF NOT CASSETTE
               lda       #CR                 GET CARRIAGE RETURN - CASSETTE ITEM SEPARATOR
LD0A3          bsr       LD0B9               SEND SEPARATOR TO CONSOLE OUT
               bra       LD07F               GET NEXT ITEM
* PRINT A STRING TO CONSOLE OUT
LD0A7          bsr       LD0B0               PRINT LEADING STRING DELIMITER (")
               jsr       >LB99F              PRINT STRING TO CONSOLE OUT
               bsr       LD0B0               PRINT ENDING STRING DELIMITER (")
               bra       LD092               GO PRINT SEPARATOR
* PRINT STRING DELIMITER (") TO CONSOLE OUT
LD0B0          jsr       >LA35F              SET PRINT PARAMETERS
               tst       PRTDEV              * GET CONSOLE PRINT DEVICE AND
               bne       LD06E               * RETURN IF CASSETTE
               lda       #'"                 QUOTE: NON-CASSETTE STRING DELIMITER
LD0B9          jmp       PUTCHR              SEND TO CONSOLE OUT
* FIELD COMMAND
FIELD          jsr       >LC82E              EVALUATE DEVICE NUMBER & VERIFY RANDOM FILE OPEN
               clra                          *
               clrb                          *	CLEAR TOTAL FIELD LENGTH COUNTER
               pshs      X,B,A               SAVE FCB POINTER & INITIALIZE TOTAL FIELD LENGTH TO ZERO
LD0C3          jsr       GETCCH              GET CURRENT INPUT CHARACTER
               bne       LD0C9               BRANCH IF NOT END OF LINE
               puls      A,B,X,PC            CLEAN UP STACK AND RETURN
LD0C9          jsr       >LB738              SYNTAX CHECK FOR COMMA, EVALUATE EXPRESSION
               pshs      X,B                 SAVE FIELD LENGTH (ACCB) ON STACK, X IS A DUMMY WHICH WILL RESERVE 2 BYTES FOR THE ADDRESS WHICH WILL BE CALCULATED BELOW
* AT THIS POINT THE STACK WILL HAVE THE FOLLOWING INFORMATION ON IT:
* ,S = FIELD LENGTH 1 2,S = RANDOM FILE BUFFER ADDRESS
* 3 4,S = TOTAL FIELD LENGTH 5 6,S = FCD POINTER
               clra                          CLEAR	MS BYTE
               addd      $03,S               ADD FIELD LENGTH TO TOTAL FIELD LENGTH COUNTER
               blo       LD0DA               'FO' ERROR IF SUM > $FFFF
               ldx       $05,S               POINT X TO FCB
               cmpd      FCBRLN,X            * COMPARE TO RECORD LENGTH & BRANCH IF
               bls       LD0DF               *TOTAL FIELD LENGTH < RECORD LENGTH
LD0DA          ldb       #34*2               'FIELD OVERFLOW' ERROR
               jmp       >LAC46              JUMP TO ERROR DRIVER
LD0DF          ldu       $03,S               LOAD U WITH OLD TOTAL LENGTH OF ALL FIELDS
               std       $03,S               SAVE NEW TOTAL FIELD LENGTH
               ldd       FCBBUF,X            POINT ACCD TO START OF RANDOM FILE BUFFER
               leau      D,U                 *POINT U TO THIS FIELD'S SLOT IN THE RANDOM
               stu       $01,S               *FILE BUFFER AND SAVE IT ON THE STACK
               ldb       #$FF                SECONDARY TOKEN
               jsr       >LB26F              SYNTAX CHECK FOR SECONDARY TOKEN
               ldb       #TOKEN_AS           'AS' TOKEN
               jsr       >LB26F              SYNTAX CHECK FOR 'AS' TOKEN
               jsr       >LB357              EVALUATE VARIABLE
               jsr       >LB146              'TM' ERROR IF NUMERIC VARIABLE
               puls      B,U                 * PULL STRING ADDRESS AND LENGTH
               stb       ,X                  * OFF OF THE STACK AND SAVE THEM
               stu       $02,X               * IN STRING DESCRIPTOR
               bra       LD0C3               CHECK FOR ANOTHER FIELD SPECIFICATION
* RSET COMMAND
RSET           fcb       $86                 SKIP ONE BYTE (THROWN AWAY LDA INSTRUCTION)
* LSET COMMAND
LSET           clra                          LSET	FLAG = 0
               pshs      A                   SAVE RSET($4F),LSET(00) FLAG ON THE STACK
               jsr       >LB357              EVALUATE FIELD STRING VARIABLE
               jsr       >LB146              'TM' ERROR IF NUMERIC VARIABLE
               pshs      X                   SAVE STRING DESCRIPTOR ON STACK
               ldx       $02,X               POINT X TO ADDRESS OF STRING
               cmpx      #DFLBUF             * COMPARE STRING ADDRESS TO START OF RANDOM
               blo       LD119               * FILE BUFFER; 'SE' ERROR IF < RANDOM FILE BUFFER
               cmpx      FCBADR              = COMPARE STRING ADDRESS TO TOP OF RANDOM FILE BUFFER
               blo       LD11E               = AREA - BRANCH IF STRING IN RANDOM FILE BUFFER
LD119          ldb       #2*35               'SET TO NON-FIELDED STRING' ERROR
               jmp       >LAC46              JUMP TO ERROR HANDLER
LD11E          ldb       #TOKEN_EQUAL        *
               jsr       >LB26F              * SYNTAX CHECK FOR '=' TOKEN
               jsr       >L8748              =EVALUATE DATA STRING EXPRESSION; RETURN WITH X
*			=POINTING TO STRING; ACCB = LENGTH
               puls      Y                   POINT Y TO FIELD STRING DESCRIPTOR
               lda       ,Y                  GET LENGTH OF FIELD STRING
               beq       LD15A               RETURN IF NULL STRING
               pshs      B                   SAVE LENGTH OF DATA STRING ON STACK
               ldb       #SPACE              PREPARE TO FILL DATA STRING WITH BLANKS
               ldu       $02,Y               POINT U TO FIELD STRING ADDRESS
* FILL THE FIELDED STRING WITH BLANKS
LD132          stb       ,U+                 STORE A SPACE IN FIELDED STRING
               deca                          DECREMENT LENGTH COUNTER
               bne       LD132               KEEP FILLING W/SPACES IF NOT DONE
               ldb       ,S+                 *GET THE LENGTH OF THE DATA STRING AND
               beq       LD15A               *RETURN IF IT IS NULL (ZERO)
               cmpb      ,Y                  =COMPARE LENGTH OF DATA STRING TO LENGTH OF FIELD
               blo       LD143               =STRING, BRANCH IF FIELD STRING > DATA STRING
               ldb       ,Y                  *GET THE LENGTH OF THE FIELD STRING AND FORCE THE
               clr       ,S                  *RSET/LSET FLAG TO LSET (0) IF DATA STRING LENGTH IS
*			*>= THE FIELD STRING LENGTH. THIS WILL CAUSE THE RIGHT
*			*SIDE OF THE DATA STRING TO BE TRUNCATED
LD143          ldu       $02,Y               LOAD U WITH THE ADDRESS OF THE FIELD STRING
               tst       ,S+                 * GET THE RSET/LSET FLAG FROM THE STACK
               beq       LD157               * AND BRANCH IF LSET
* RSET ROUTINE
               pshs      B                   SAVE THE NUMBER OF BYTES TO MOVE INTO THE FIELD STRING
               clra                          = TAKE THE 2'S COMPLEMENT OF AN UNSIGNED
               negb                          = NUMBER IN ACCB - LEAVE THE DOUBLE BYTE SIGNED
               sbca      #$00                = RESULT IN ACCD
               addb      ,Y                  * ADD THE LENGTH OF THE FIELD STRING TO THE INVERSE
               adca      #$00                * OF THE NUMBER OF BYTES TO BE MOVED
               leau      D,U                 =ADD RESULT TO START OF FIELD STRING. NOW U
* =WILL POINT TO (-NUMBER OF BYTES TO MOVE)
* =FROM THE RIGHT SIDE OF THE FIELD STRING
               puls      B                   GET THE NUMBER OF BYTES TO MOVE
LD157          jmp       >LA59A              MOVE ACCB BYTES FROM X TO U (DATA TO FIELD STRING)
LD15A          puls      A,PC                PULL LSET/RSET FLAG OFF OF STACK AND RETURN
* FILES COMMAND
FILES          jsr       >L95AC              RESET SAM DISPLAY PAGE AND VDG MODE
               ldd       FCBADR              GET START OF FILE BUFFERS
               subd      #DFLBUF             SUBTRACT THE START OF RANDOM FILE BUFFER SPACE
               pshs      B,A                 SAVE DEFAULT VALUE OF RANDOM FILE BUFFER SPACE ON STACK
               ldb       FCBACT              * GET CURRENT NUMBER OF FCBS
               pshs      B                   * AND SAVE ON THE STACK (DEFAULT VALUE)
               jsr       GETCCH              GET CURRENT INPUT CHAR
               cmpa      #',                 CHECK FOR COMMA
               beq       LD181               BRANCH IF COMMA - NO BUFFER NUMBER PARAMETER GIVEN
               jsr       EVALEXPB            EVALUATE EXPRESSION (BUFFER NUMBER)
               cmpb      #15                 15 FCBS MAX
               lbhi      LB44A               BRANCH IF > 15 - 'ILLEGAL FUNCTION CALL'
               stb       ,S                  SAVE NUMBER OF FCBS ON STACK
               jsr       GETCCH              CHECK CURRENT INPUT CHAR
               beq       LD189               BRANCH IF END OF LINE
LD181          jsr       SYNCOMMA            SYNTAX CHECK FOR COMMA
               jsr       >LB3E6              EVALUATE EXPRESSION, RETURN VALUE IN ACCD
               std       $01,S               SAVE RANDOM FILE BUFFER SIZE ON STACK
LD189          jsr       DVEC7               CLOSE FILES
               ldb       ,S                  * GET THE NUMBER OF BUFFERS TO MAKE AND
               pshs      B                   * INITIALIZE A BUFFER COUNTER ON THE STACK
               ldd       #DFLBUF             GET START OF RANDOM FILE BUFFERS
               addd      $02,S               ADD THE NEWLY SPECIFIED RANDOM FILE BUFFER SPACE
               blo       LD208               'OUT OF MEMORY' ERROR IF > $FFFF
               std       $02,S               SAVE START OF FCBS
* RESERVE SPACE FOR FCBS
LD199          addd      #FCBLEN             FCBLEN REQUIRED FOR EACH BUFFER
               blo       LD208               'OUT OF MEMORY' ERROR IF > $FFFF
               dec       ,S                  DECREMENT BUFFER COUNTER
               bpl       LD199               *BRANCH IF NOT DONE - THE BPL WILL SET UP ONE MORE BUFFER
* *THAN THE NUMBER REQUESTED. THIS EXTRA BUFFER IS THE SYSTEM BUFFER
* *AND IS LOCATED AT THE END OF THE NORMAL FCBS. ONLY SYSTEM ROUTINES
* *(COPY, BACKUP, MERGE ETC.) MAY ACCESS THIS BUFFER.
               tstb                          AT AN EXACT 256 BYTE BOUNDARY?
               beq       LD1A8               YES
               inca                          NO - ADD 256
               beq       LD208               'OUT OF MEMORY' ERROR IF PAST $FFFF
LD1A8          bita      #$01                ON A 512 BYTE BOUNDARY?
               beq       LD1AF               YES
               inca                          NO - ADD 256
               beq       LD208               'OM' ERROR IF PAST $FFFF
LD1AF          sta       ,S                  SAVE MS BYTE OF NEW GRAPHIC RAM START
               ldd       VARTAB              GET START OF VARIABLES
               suba      GRPRAM              *SUBTRACT THE OLD GRAPHIC RAM START - ACCD CONTAINS LENGTH
* *OF PROGRAM PLUS RESERVED GRAPHIC RAM
               adda      ,S                  ADD IN THE AMOUNT OF RAM CALCULATED ABOVE
               blo       LD208               'OUT OF MEMORY' ERROR IF > $FFFF
               tfr       D,X                 SAVE NEW VARTAB IN X
               inca                          *ADD 256 - TO GUARANTEE ENOUGH ROOM SINCE ALL CALCULATIONS USE
* *ONLY THE MSB OF THE ADDRESS
               beq       LD208               'OUT OF MEMORY' ERROR IF PAST $FFFF
               cmpd      FRETOP              IS IT GREATER THAN THE START OF STRING SPACE
               bhs       LD208               'OUT OF MEMORY' IF > START OF STRING SPACE
               deca                          SUBTRACT 256 - COMPENSATE FOR INCA ABOVE
               subd      VARTAB              SUBTRACT START OF VARIABLES
               addd      TXTTAB              ADD START OF BASIC
               tfr       D,Y                 Y HAS NEW START OF BASIC
               lda       ,S                  * GET THE GRAPHIC RAM START, SUBTRACT
               suba      GRPRAM              * THE OLD GRAPHIC RAN START AND SAVE
               tfr       A,B                 * THE DIFFERENCE IN ACCA AND ACCB
               adda      BEGGRP              = ADD THE OLD GRAPHIC PAGE START AND
               sta       BEGGRP              = STORE THE NEW START OF GRAPHICS RAM
               addb      ENDGRP              * ADD THE OLD GRAPHIC RAM END ADDRESS AND
               stb       ENDGRP              * STORE THE NEW END OF GRAPHICS RAM
               puls      A,B,U               = ACCA=MSB OF START OF GRAPHIC RAM; ACCB=NUMBER OF FILE BUFFERS
* = U=START OF FILE BUFFERS
               sta       GRPRAM              SAVE NEW START OF GRAPHIC RAM
               stb       FCBACT              NUMBER OF FILE BUFFERS
               stu       FCBADR              START OF FILE BUFFERS
               lda       CURLIN              GET CURRENT LINE NUMBER
               inca                          ARE WE IN DIRECT MODE?
               beq       LD1EF               YES - MOVE BASIC PROGRAM
               tfr       Y,D                 MOVE NEW START OF BASIC TO ACCD
               subd      TXTTAB              SUBTRACT OLD START OF BASIC
               addd      CHARAD              ADD OLD INPUT POINTER
               std       CHARAD              SAVE NEW INPUT POINTER
LD1EF          ldu       VARTAB              POINT U TO OLD START OF VARIABLES
               stx       VARTAB              SAVE NEW START OF VARIBLES
               cmpu      VARTAB              * COMPARE OLD START OF VARIABLES TO NEW START OF
               bhi       LD20B               * VARIABLES & BRANCH IF OLD > NEW
* MOVE BASIC PROGRAM IF OLD START ADDRESS <= NEW START ADDRESS
LD1F8          lda       ,-U                 GET A BYTE
               sta       ,-X                 MOVE lT
               cmpu      TXTTAB              AT START OF BASIC PROGRAM?
               bne       LD1F8               NO
               sty       TXTTAB              STORE NEW START OF BASIC PROGRAM
               clr       -1,Y                RESET START OF PROGRAM FLAG
               bra       LD21B               CLOSE ALL FILES
LD208          jmp       >LAC44              'OUT OF MEMORY' ERROR
* MOVE BASIC PROGRAM IF OLD START ADDRESS > NEW START ADDRESS
LD20B          ldu       TXTTAB              POINT U TO OLD START OF BASIC
               sty       TXTTAB              SAVE NEW START OF BASIC
               clr       -1,Y                RESET START OF BASIC FLAG
LD212          lda       ,U+                 GET A BYTE
               sta       ,Y+                 MOVE IT
               cmpy      VARTAB              AT START OF VARIABLES
               bne       LD212               NO - MOVE ANOTHER BYTE
* CLOSE ALL FCBS AND RECALCULATE FCB START ADDRESSES
LD21B          ldu       #FCBV1              POINT U TO FILE BUFFER POINTERS
               ldx       FCBADR              POINT X TO START OF BUFFERS
               clrb                          RESET	FILE COUNTER
LD222          stx       ,U++                STORE FILE ADDRESS IN VECTOR TABLE
               clr       FCBTYP,X            RESET FILE TYPE TO CLOSED
               leax      FCBLEN,X            GO TO NEXT FCB
               incb                          INCREMENT FILE COUNTER
               cmpb      FCBACT              CLOSE ALL ACTIVE BUFFERS AND SYSTEM FCB
               bls       LD222               BRANCH IF NOT DONE
               jmp       >L96CB              READJUST LINE NUMBERS, ETC.
* UNLOAD COMMAND
UNLOAD         bsr       LD24F               GET DRIVE NUMBER
               clrb                          CLEAR	FILE COUNTER
LD236          incb                          INCREMENT FILE COUNTER
               jsr       >LC749              POINT X TO FCB
               beq       LD249               BRANCH IF FILE NOT OPEN
               lda       FCBDRV,X            CHECK DRIVE NUMBER
               cmpa      DCDRV               DOES IT MATCH THE 'UNLOAD' DRIVE NUMBER?
               bne       LD249               NO MATCH - DO NOT CLOSE THE FILE
               pshs      B                   SAVE FILE COUNTER ON THE STACK
               jsr       >LCB06              CLOSE FCB
               puls      B                   RESTORE FILE COUNTER
LD249          cmpb      FCBACT              CHECKED ALL FILES?
               bls       LD236               NO
               rts       
* GET DRIVE NUMBER FROM BASIC - USE THE DEFAULT DRIVE IF NONE GIVEN
LD24F          ldb       DEFDRV              GET DEFAULT DRIVE NUMBER
LD252          jsr       GETCCH              GET NEXT INPUT CHAR
               beq       LD25F               USE DEFAULT DRIVE NUMBER IF NONE GIVEN
LD256          jsr       EVALEXPB            EVALUATE EXPRESSION
               jsr       >HITEST
               nop       
               nop       
               nop       
*	CMPB	#$03	4 DRIVES MAX
*	LBHI	LA61F	'DEVICE NUMBER ERROR' IF > 3
LD25F          stb       DCDRV               STORE IN DSKCON VARIABLE
LD261          rts       
* BACKUP COMMAND
BACKUP         lbeq      LA61F               DEVICE NUMBER ERROR IF NO DRIVE NUMBERS GIVEN
               jsr       >L95AC              RESET SAM DISPLAY PAGE AND VOG MODE
               jsr       >LD256              * GET SOURCE DRIVE NUMBER AND SAVE
               pshs      B
               nop       
*	STB	DBUF0+255	* IT AT TOP OF DBUF0 (TOP OF NEW STACK)
               jsr       GETCCH              GET A CHARACTER FROM BASIC
               beq       LD27B               BRANCH IF END OF LINE
               ldb       #TOKEN_TO
               jsr       >LB26F              SYNTAX CHECK FOR 'TO'
               jsr       >LD256              GET DESTINATION DRIVE NUMBER

LD27B                    
               pshs      B                   Save destination drive
               jsr       LA5C7               ?SN ERROR if more chars
               jsr       >HCLOSE             Close all disk files
               clr       ,-s                 Setup a track counter
               lda       #35                 Copy 35 tracks
               clrb                          staRTING   From zero
               pshs      D                   Save on stack
               leax      -256,s              Leave room for stack
a@             inc       2,s                 Bump up track count
               leax      -(18*256),x         Subtract 1 track
               cmpx      <$1F                Bottom of RAM yet?
               bhs       a@                  No, add another track
               dec       2,s                 Normalize to zero
               lbeq      LAC44               ?OM ERROR if no free RAM
               bra       LD2A4
PD2A0          puls      cc,dp,d,x,y,u
               bra       LD261

*LD27B	LDS	#DBUF0+255	PUT STACK AT TOP OF DBUF0
*	PSHS	B	SAVE DESTINATION DRIVE NUMBER ON STACK
*	JSR	>LA5C7	SYNTAX ERROR IF NOT END OF LINE
*	JSR	DVEC7	CLOSE ALL FILES
*	CLR	,-S	CLEAR A TRACK COUNTER ON STACK
*	LDX	#DFLBUF-1	POINT X TO TOP OF DISK RAM VARIABLES
*LD28C	INC	,S	INCREMENT TRACK COUNTER
*	LEAX	SECMAX*SECLEN,X	INCREMENT X BY ONE TRACK
*	CMPX	MEMSIZ	COMPARE TO TOP OF NON RESERVED RAN
*	BLS	LD28C	KEEP GOING IF MORE FREE RAM LEFT
*	DEC	,S	DECREMENT TRACK COUNTER
*	LBEQ	LAC44	'OM' ERROR IF < 1 TRACK OF FREE RAM
*	LDA	#TRKMAX	GET MAXIMUM NUMBER OF TRACKS — INITIALIZE REMAINING TRACKS CTR
*	CLRB	INITIALIZE	TRACKS WRITTEN COUNTER TO ZERO
*	PSHS	B,A	SAVE TRACKS WRITTEN AND REMAINING COUNTERS ON STACK
* AT THIS POINT THE STACK HAS THE FOLLOWING DATA ON IT:
* ,S = TRACKS REMAINING COUNTER; 1,S = TRACKS WRITTEN COUNTER
* 2,S = NUMBER OF TRACKS WHICH FIT IN RAM; 3,S = DESTINATION DRIVE NUMBER
* 4,S = SOURCE DRIVE NUMBER
*	COM	DRESFL	SET THE DISK RESET FLAG TO CAUSE A RESET
LD2A4          clrb                          INITIALIZE	WRITE TRACK COUNTER TO ZERO
LD2A5          incb                          ADD ONE TO WRITE TRACK COUNTER
               dec       ,S                  * DECREMENT REMAINING TRACKS COUNTER
               beq       LD2AE               * AND BRANCH IF NO TRACKS LEFT
               cmpb      $02,S               = COMPARE WRITE TRACK COUNTER TO NUMBER OF TRACKS THAT
               bne       LD2A5               = WILL FIT IN RAM AND BRANCH IF ROOM FOR MORE TRACKS IN RAM
LD2AE          stb       TMPLOC              SAVE THE NUMBER OF TRACKS TO BE TRANSFERRED
               ldb       $04,S               GET SOURCE DRIVE NUMBER
               bsr       LD2FC               FILL RAM BUFFER WITH TMPLOC TRACKS OF DATA
               lda       #$FF                SET SOURCE/DESTINATION FLAG TO DESTINATION
               jsr       >LD322              PRINT PROMPT MESSAGE IF NEEDED
               ldb       $03,S               GET DESTINATION DRIVE NUMBER
               bsr       LD2FF               WRITE TMPLOC TRACKS FROM BUFFER
               tst       ,S                  TEST TRACKS REMAINING FLAG
               beq       LD2CD               BRANCH IF BACKUP DONE
               clra                          SET	SOURCE/DESTINATION FLAG TO SOURCE
               jsr       >LD322              PRINT PROMPT MESSAGE IF NEEDED
               ldb       $01,S               * GET THE TRACKS WRITTEN COUNTER, ADD THE NUMBER OF
               addb      TMPLOC              * TRACKS MOVED THIS TIME THROUGH LOOP AND
               stb       $01,S               * SAVE THE NEW TRACKS WRITTEN COUNTER
               bra       LD2A4               COPY SOME MORE TRACKS
LD2CD          leas      5,s
               jmp       >BEEP
*LD2CD	BSR	LD2D2	CHECK FOR DOS INITIALIZATION
*	JMP	>LAC73	JUMP BACK TO BASIC’S MAIN LOOP
LD2D2          puls      U                   PUT THE RETURN ADDRESS IN U
               lda       DRESFL              TEST DISK RESET FLAG
               beq       LD2EF               DON’T RESET THE DOS IF FLAG NOT SET
               ldx       #FCBV1              POINT X TO TABLE OF FCB ADDRESSES
               clra                          SET	FILE COUNTER TO ZERO
LD2DD          clr       [,X++]              MARK FCB AS CLOSED
               inca                          ADD ONE TO FILE COUNTER
               cmpa      FCBACT              COMPARE TO NUMBER OF RESERVED FILES
               bls       LD2DD               BRANCH IF ANY FILES NOT SHUT DOWN
               ldx       TXTTAB              LOAD X WITH THE START OF BASIC
               clr       -1,X                SET FIRST BYTE OF BASIC PROGRAM TO ZERO
               jsr       >LAD19              GO DO A 'NEW'
               clr       DRESFL              RESET THE DOS RESET FLAG
LD2EF          lda       DLODFL              * CHECK THE LOAD RESET FLAG AND
               beq       LD2FA               * BRANCH IF NOT SET
               clr       DLODFL              CLEAR THE LOAD RESET FLAG
               jsr       >LAD19              GO DO A 'NEW'
LD2FA          jmp       ,U                  JUMP BACK TO RETURN ADDRESS SAVED IN U ABOVE
LD2FC          lda       #$02                READ OP CODE
               fcb       $8C                 SKIP TWO BYTES (THROWN AWAY CMPX INSTRUCTION)
LD2FF          lda       #$03                WRITE OP CODE
               std       DCOPC               SAVE IN DSKCON VARIABLE
               lda       $03,S               * GET THE NUMBER OF THE TRACK BEING CURRENTLY
               sta       DCTRK               * WRITTEN AND SAVE IT IN DSKCON VARIABLE
               ldx       >$1F                = TRACK BUFFER STARTS AT DFLBUF
*	LDX	#DFLBUF	= TRACK BUFFER STARTS AT DFLBUF
               stx       DCBPT               = SAVE IT IN DSKCON VARIABLE
               lda       TMPLOC              GET NUMBER OF TRACKS TO MOVE
LD30E          ldb       #$01                INITIALIZE SECTOR COUNTER TO ONE
LD310          stb       DSEC                SAVE DSKCON SECTOR VARIABLE
               jsr       >LD6F2              READ/WRITE A SECTOR
               inc       DCBPT               MOVE BUFFER POINTER UP ONE SECTOR (256 BYTES)
               incb                          INCREMENT SECTOR COUNTER
               cmpb      #SECMAX             COMPARE TO MAXIMUM NUMBER OF SECTORS PER TRACK
               bls       LD310               BRANCH IF ANY SECTORS LEFT
               inc       DCTRK               INCREMENT TRACK COUNTER VARIABLE TO NEXT TRACK
               deca                          DECREMENT TRACKS TO MOVE COUNTER
               bne       LD30E               READ MORE TRACKS IF ANY LEFT
               rts       
LD322          ldb       $05,S               * GET THE DESTINATlON DRIVE NUMBER AND
               cmpb      $06,S               * COMPARE IT TO THE SOURCE DRIVE NUMBER
* PRINT SOURCE/DESTINATION DISK SWITCH PROMPT MESSAGE
LD326          bne       LD35E               RETURN IF DRIVE NUMBERS NOT EQUAL
               clr       RDYTMR              RESET THE READY TIMER
               clr       DSKREG              CLEAR DSKREG - TURN OFF ALL DISK MOTORS
               clr       DRGRAM              CLEAR DSKREG RAM IMAGE
               pshs      A                   SAVE SOURCE/DESTINATION FLAG ON STACK
               jsr       >CLS
*	JSR	>LA928	CLEAR SCREEN
               ldx       #LD35F              POINT X TO 'INSERT SOURCE' MESSAGE
               ldb       #13                 13 BYTES IN MESSAGE
               lda       ,S+                 GET SOURCE/DESTINATION FLAG FROM THE STACK
               beq       LD344               BRANCH IF SOURCE
               ldx       #LD36C              POINT X TO 'INSERT DESTINATION' MESSAGE
               ldb       #18                 18 BYTES IN MESSAGE
LD344          jsr       >LB9A2              SEND MESSAGE TO CONSOLE OUT
               ldx       #LD37E              POINT X TO 'DISKETTE AND' MESSAGE
               ldb       #27                 27 BYTES IN MESSAGE
               jsr       >LB9A2              SEND MESSAGE TO CONSOLE OUT
               jsr       >BEEP
               jsr       >BEEP
               nop       
               nop       
*	LDD	#$6405	* SET UP 'SOUND' PARAMETERS
*	STA	SNDTON	* FOR A BEEP
*	JSR	>LA951	JUMP TO 'SOUND' - DO A BEEP
LD357          jsr       >LA171              GET A CHARACTER FROM CONSOLE IN
               cmpa      #CR                 * KEEP LOOKING AT CONSOLE IN UNTIL
               bne       LD357               * YOU GET A CARRIAGE RETURN
LD35E          rts       
LD35F          fcc       'INSERT SOURCE'
LD36C          fcc       'INSERT DESTINATION'
LD37E          fcc       ' DISKETTE AND'
               fcb       CR
               fcc       "PRESS 'ENTER'"
* PUSH FILENAME.EXT AND DRIVE NUMBER ONTO THE STACK
LD399          puls      Y                   SAVE RETURN ADDRESS IN Y
               ldb       #11                 11 CHARACTERS IN FILENAME AND EXTENSION
               ldx       #DNAMBF+11          POINT X TO TOP OF DISK NAME/EXT BUFFER
LD3A0          lda       ,-X                 * GET A CHARACTER FROM FILENAME.
               pshs      A                   * EXT BUFFER AND PUSH IT ONTO THE
               decb                          * STACK - DECREMENT COUNTER AND
               bne       LD3A0               * KEEP LOOPING UNTIL DONE
               lda       DCDRV               = GET DRIVE NUMBER AND PUSH
               pshs      A                   = IT ONTO THE STACK
               jmp       ,Y                  PSEUDO - RETURN TO CALLING ROUTINE
* PULL FILENAME.EXT AND DRIVE NUMBER FROM (X) TO RAM
LD3AD          lda       ,X+                 * GET DRIVE NUMBER AND SAVE
               sta       DCDRV               * IT IN DSKCON VARIABLE
LD3B1          ldb       #11                 11 BYTES IN FILENAME AND EXTENSION
               ldu       #DNAMBF             POINT U TO DISK NAME BUFFER
               jmp       >LA59A              MOVE FILENANE.EXT FROM (X) TO DNAMBF
* COPY
* THE COPY PROCESS IS PERFORMED BY COPYING DATA FROM THE SOURCE FILE
* TO RAM AND THEN COPYING IT TO THE DESTINATION FILE. THE SOURCE AND
* DESTINATION FILES ARE OPENED AS RANDOM FILES AND BOTH USE THE SYSTEM
* FCB ABOVE THE RESERVED FCBS. ALL OF AVAILABLE FREE RAM ABOVE THE
* VARIABLES IS USED AS A COPY BUFFER WHICH SPEEDS UP THE COPYING PROCESS
* BUT UNFORTUNATELY THE METHOD USED WILL ALLOW AN ERROR ENCOUNTERED DURING
* THE COPY PROCESS TO 'HANG' THE SYSTEM. THIS IS CAUSED BY POINTING THE FCB'S
* RANDOM FILE BUFFER POINTER (FCBBUF,X) TO THE FREE RAM BUFFER. AN ERROR
* WILL THEN CAUSE THE OPEN FILE TO BE CLOSED WITH FCBBUF,X POINTING TO AN
* AREA IN RAM WHERE THE RANDOM FILE BUFFER CLOSE ROUTINE (LCAE2) WILL NEVER
* LOOK FOR IT
COPY           jsr       >LC935              * GET SOURCE FILENAME.EXT & DRIVE NUMBER FROM BASIC
               bsr       LD399               * AND SAVE THEM ON THE STACK
               clr       ,-S                 CLEAR A BYTE ON STACK - SINGLE DISK COPY (SDC) FLAG
               jsr       GETCCH              GET CURRENT INPUT CHARACTER
               beq       LD3CE               BRANCH IF END OF LINE - SINGLE DISK COPY
               com       ,S                  SET SOC FLAG TO $FF (NO SINGLE DISK COPY)
               ldb       #TOKEN_TO
               jsr       >LB26F              SYNTAX CHECK FOR 'TO'
               jsr       >COPtst
*	JSR	>LC935	GET DESTINATION FILENAME.EXT AND DRIVE NUMBER
LD3CE          bsr       LD399               SAVE DESTINATION FILENAME.EXT & DRIVE NUMBER ON STACK
               jsr       >LA5C7              SYNTAX ERROR IF MORE CHARACTERS ON LINE
               jsr       DVEC7               CLOSE ALL FILES
* COUNT THE NUMBER OF SECTORS WORTH OF FREE RAM AVAILABLE
               clr       ,-S                 CLEAR A SECTOR COUNTER ON THE STACK
               leax      -SECLEN,S           POINT X ONE SECTOR LENGTH DOWN FROM THE TOP OF STACK
LD3DC          inc       ,S                  INCREMENT SECTOR COUNTER
               leax      -SECLEN,X           DECREMENT X BY ONE SECTOR
               cmpx      ARYEND              COMPARE TO TOP OF ARRAYS
               bhs       LD3DC               BRANCH IF NOT AT BOTTOM OF FREE RAM
               dec       ,S                  DECREMENT SECTOR COUNTER
               lbeq      LAC44               'OM' ERROR IF NOT AT LEAST ONE FULL SECTOR OF FREE RAM
               leax      14,S                POINT X TO START OF SOURCE DATA
               bsr       LD3AD               PUT SOURCE DATA INTO DNAMBF AND DSKCON
               jsr       >LC68C              SCAN DIRECTORY FOR A MATCH
               jsr       >LC6E5              'NE' ERROR IF MATCH NOT FOUND
               ldx       V974                POINT X TO DIRECTORY RAM IMAGE OF FOUND FILE
               ldu       DIRLST,X            * GET NUMBER OF BYTES IN LAST SECTOR AND
               ldx       DIRTYP,X            * SOURCE FILE TYPE AND ASCII FLAG
               pshs      U,X                 * AND SAVE THEM ON THE STACK
               jsr       >LC79D              GET VALID FAT DATA
               ldb       V976                GET NUMBER OF FIRST GRANULE IN FILE
               jsr       >LCD1E              * GET THE NUMBER OF GRANULES IN FILE
               pshs      A                   * AND SAVE IT ON THE STACK
               deca                          SUBTRACT OFF THE LAST GRANULE
               andb      #$3F                * MASK OFF LAST GRANULE FLAG BITS AND SAVE THE
               pshs      B                   * NUMBER OF SECTORS IN LAST GRANULE ON STACK
               tfr       A,B                 SAVE THE NUMBER OF GRANULES IN ACCB
               clra                          CLEAR	THE MS BYTE OF ACCD
               jsr       >LC779              MULTIPLY ACCD BY NINE
               addb      ,S                  * ADD THE NUMBER OF SECTORS IN THE LAST
               adca      #$00                * GRANULE TO ACCD
               ldx       #$0001              INITIALIZE RECORD COUNTER TO ONE
               pshs      X,B,A               INITIALIZE SECTOR AND RECORD COUNTERS ON THE STACK
* AT THIS POINT THE CONTROL VARIABLES FOR COPY ARE STORED ON THE STACK.
* 0 1,S = REMAINING SECTORS COUNTER; 2 3,S = RECORD COUNTER
* 4,S = NUMBER OF SECTORS TO BE COPIED. INITIALLY SET TO NUMBER OF
* SECTORS IN THE LAST GRANULE.
* 5,S = GRAN TEST FLAG. INITIALLY SET TO NUMBER OF GRANS IN FILE
* 6,S = FILE TYPE; 7,S = ASCII FLAG; 8 9,S = NUMBER OF BYTES IN LAST SECTOR
* 10,S = NUMBER OF SECTORS WHICH WILL FIT IN THE CURRENTLY AVAILABLE FREE RAM
* 11-22,S = DESTINATION FILENAME.EXT AND DRIVE NUMBER
* 23,S = SINGLE DISK COPY FLAG; 24-35,S = SOURCE FILENAME.EXT AND DRIVE NUMBER
LD41E          clrb                          SET	SECTOR COUNTER TO ZERO
               ldx       ,S                  GET THE NUMBER OF SECTORS REMAINING IN THE FILE
               beq       LD42C               BRANCH IF NO SECTORS LEFT
LD423          incb                          ADD A SECTOR TO TEMPORARY SECTOR COUNTER
               leax      -1,X                DECREMENT REMAINING SECTORS COUNTER
               beq       LD42C               BRANCH IF NO SECTORS LEFT
               cmpb      10,S                *COMPARE TEMPORARY COUNTER TO NUMBER OF SECTORS WHICH MAY
* *BE STORED IN FREE RAM
               bne       LD423               BRANCH IF STILL ROOM FOR MORE SECTORS
LD42C          stx       ,S                  SAVE THE NUMBER OF UNCOPIED SECTORS REMAINING IN THE FILE
               stb       $04,S               SAVE THE NUMBER OF SECTORS TO BE COPIED THIS TIME THROUGH LOOP
               bsr       LD482               'GET' ACCB SECTORS TO RAM BUFFER
               lda       #$FF                SET SOURCE/DESTINATION FLAG TO DESTINATION
               bsr       LD476               PRINT PROMPT MESSAGE IF REQUIRED
               tst       $05,S               * CHECK THE GRAN TEST FLAG. IF <> 0, IT CONTAINS THE
               beq       LD45F               * NUMBER OF GRANS IN THE FILE AND THE DESTINATION DISK
* * MUST BE CHECKED FOR ENOUGH ROOM. IF IT IS =0
* * THEN THE CHECK HAS ALREADY BEEN DONE
               leax      11,S                POINT TO DESTINATION FILE PARAMETERS
               jsr       >LD3AD              GET DESTINATION FILE PARAMETERS FROM STACK
               jsr       >AETEST
*	JSR	>LD059	SCAN DIRECTORY FOR FILE - 'AE' ERROR IF IT EXISTS
               jsr       >LC79D              GET VALID FAT DATA
* MAKE SURE THERE ARE ENOUGH FREE GRANULES ON THE DESTINATION DISK
               jsr       >LC755              POINT X TO FAT
               leax      FATCON,X            SKIP PAST THE FAT CONTROL BYTES
               lda       $05,S               GET THE NUMBER OF GRANS IN THE FILE
               ldb       #GRANMX             SET GRAN COUNTER TO MAXIMUM
LD44E          com       ,X                  * CHECK TO SEE IF A BRAN IS FREE
               bne       LD455               * AND BRANCH IF IT IS NOT FREE
               deca                          = DECREMENT COUNTER AND BRANCH IF
               beq       LD45D               = THERE ARE ENOUGH FREE GRANULES
LD455          com       ,X+                 RESTORE FAT BYTE AND INCREMENT POINTER
               decb                          DECREMENT GRAN COUNTER
               bne       LD44E               BRANCH IF ALL GRANS NOT CHECKED
               jmp       >LC7F8              'DISK FULL' ERROR
LD45D          com       ,X                  RESTORE FAT BYTE
LD45F          bsr       LD47C               'PUT' DATA FROM RAM BUFFER TO DESTINATION FILE
               ldx       ,S                  GET THE NUMBER OF REMAINING SECTORS
               beq       LD472               EXIT ROUTINE IF NO SECTORS LEFT
               ldd       $02,S               *
               addb      $04,S               * GET THE CURRENT RECORD COUNTER, ADD
               adca      #$00                * THE NUMBER OF SECTORS (RECORDS) MOVED
               std       $02,S               * AND SAVE THE NEW RECORD COUNTER
               clra                          SET	SOURCE/DESTINATION FLAG TO SOURCE
               bsr       LD476               PRINT PROMPT MESSAGE IF REQUIRED
               bra       LD41E               KEEP COPYING SECTORS
LD472          leas      36,S                REMOVE TEMPORARY STORAGE VARIABLES FROM STACK
               rts                           **** COPY DONE ****
LD476          tst       25,S                *CHECK SINGLE DISK COPY FLAG - IF <> ZERO, THEN DON'T
* *PRINT THE PROMPT MESSAGE
               jmp       >LD326              PRINT THE PROMPT MESSAGE IF REQUIRED
* 'PUT'.'GET' DATA FROM THE DESTINATION/SOURCE FILES
LD47C          lda       #$FF                'PUT' FLAG
               leax      13,S                POINT X TO DESTINATION FILENAME DATA
               bra       LD486               GO 'PUT' SOME DATA
LD482          clra                          ZERO	IS THE 'GET' FLAG
               leax      26,S                POINT X TO THE SOURCE FILENAME DATA
LD486          sta       VD8                 SAVE THE 'GET'/'PUT' FLAG
               jsr       >LD3AD              GET FILENAME AND DRIVE DATA FROM THE STACK
               ldx       $08,S               * GET ASCII FLAG AND FILE TYPE AND SAVE
               stx       DFLTYP              * THEM IN THE DISK RAM VARIABLES
               ldx       #SECLEN             = SAVE ONE SECTOR LENGTH IN
               stx       DFFLEN              = RAM RECORD LENGTH VARIABLE
               lda       #'R                 RANDOM FILE TYPE FLAG
               ldb       FCBACT              * GET THE HIGHEST RESERVED FCB NUMBER, ADD ONE
               incb                          * AND OPEN A RANDOM FILE WHOSE FCB WILL BE ONE ABOVE
               jsr       >LC48D              * THE HIGHEST RESERVED FCB (THE SYSTEM FCB)
               ldx       FCBTMP              POINT X TO THE 'SYSTEM' FCB
               ldd       #SECLEN             * SET THE NUMBER OF BYTES IN THE LAST SECTOR
               std       FCBLST,X            * OF THE FILE EQUAL TO ONE SECTOR LENGTH
               ldb       $06,S               =GET THE NUMBER OF SECTORS TO MOVE AND
               beq       LD4D4               =BRANCH IF NONE LEFT
               ldb       VD8                 *GRAB THE 'GET'/'PUT' FLAG, 'AND' IT WITH THE
               andb      $07,S               *GRAN TEST FLAG - BRANCH IF 'GET'ING DATA OR THIS IS
               beq       LD4BA               *NOT THE FIRST TIME THROUGH THE LOOP
               ldd       $02,S               =GET THE NUMBER OF SECTORS REMAINING TO BE COPIED AND
               addb      $06,S               =ADD THE NUMBER TO BE COPIED THIS TIME THROUGH LOOP
               adca      #$00                =
               jsr       >LC2E6              *'PUT' THE LAST RECORD IN THE FILE TO THE SYSTEM FCB.
*			*THE RECORD NUMBER IS IN ACCD.
LD4BA          ldx       FCBTMP              POINT X TO THE SYSTEM FCB
               ldu       $04,S               * GET THE CURRENT RECORD NUMBER
               stu       FCBREC,X            * AND SAVE IT IN THE FCB
               ldb       $06,S               GET THE NUMBER OF THE RECORD (SECTOR) TO MOVE
               ldu       ARYEND              END OF ARRAYS IS THE START OF THE COPY FREE RAM BUFFER
LD4C4          pshs      U,B                 SAVE SECTOR COUNTER AND BUFFER POINTER ON THE STACK
               ldx       FCBTMP              POINT X TO SYSTEM FCB
               stu       FCBBUF,X            *SET THE RANDOM FILE BUFFER POINTER TO THE 'COPY' RAM BUFFER
*			*THIS WILL CAUSE THE SYSTEM TO 'HANG' IF AN ERROR OCCURS DURING COPY.
               jsr       >LC2EA              GO 'GET' OR 'PUT' DATA TO THE SYSTEM FCB
               inc       $01,S               ADD 256 (ONE SECTOR) TO THE BUFFER POINTER
               puls      B,U                 GET THE SECTOR COUNTER AND BUFFER POINER
               decb                          DECREMENT SECTOR COUNTER
               bne       LD4C4               BRANCH IF ALL SECTORS NOT DONE
LD4D4          ldx       FCBTMP              POINT X TO SYSTEM FCB
               ldu       #DFLBUF             * RESET THE RANDOM FILE BUFFER POINTER FOR THE SYSTEM
               stu       FCBBUF,X            * FCB TO THE BOTTOM OF RANDOM FILE BUFFER AREA
               ldb       VD8                 =GRAB THE 'GET'/'PUT' FLAG, 'AND' IT WITH THE GRAN
               andb      $07,S               =TEST FLAG - CLOSE THE FILE IF 'GET'ING DATA AND
               beq       LD4EA               =THIS IS NOT THE FIRST TIME THROUGH THE LOOP
               clr       $07,S               RESET THE GRAN TEST FLAG IF FIRST TIME THROUGH LOOP
               ldd       10,S                *GET THE NUMBER OF BYTES IN THE LAST SECTOR,
               ora       #$80                *'OR' IN THE PRE-SAVED FLAG AND
               std       FCBLST,X            *SAVE THE NUMBER OF BYTES IN THE LAST SECTOR IN THE FCB
LD4EA          jmp       >LCB06              CLOSE THE FILE
* DSKI$ COMMAND
DSKI           bsr       LD527               GET THE DRIVE, TRACK AND SECTOR NUMBERS
               bsr       LD51C               * EVALUATE STRING VARIABLE 1 AND SAVE
               pshs      X                   * THE DESCRIPTOR ADDRESS ON THE STACK
               bsr       LD51C               = EVALUATE STRING VARIABLE 2 AND SAVE
               pshs      X                   = THE DESCRiPTOR ADDRESS ON THE STACK
               ldb       #$02                DSKCON READ OP CODE
               jsr       >LD58F              REAO A SECTOR INTO DBUF0
               ldu       #DBUF0+128          POINT U TO TOP HALF OF DBUF0
               puls      X                   GET STRING 2 DESCRIPTOR ADDRESS
               bsr       LD508               PUT STRING 2 INTO STRING SPACE
               ldu       #DBUF0              POINT U TO BOTTOM HALF OF DBUF0
               puls      X                   GET STRING 1 DESCRIPTOR ADDRESS
LD508          pshs      U,X                 PUT STRING DESCRIPTOR & SOURCE POINTER ON THE STACK
               ldb       #128                *
               jsr       >LB50F              * RESERVE 128 BYTES IN STRING SPACE
               leau      ,X                  POINT U TO RESERVED STRING SPACE
               puls      X                   GET STRING DESCRIPTOR ADDRESS
               stb       ,X                  * SAVE DESCRIPTOR DATA (LENGTH AND ADDRESS)
               stu       $02,X               * OF THE NEW STRING
               puls      X                   GET THE SOURCE (DBUF0) POINTER
LD519          jmp       >LA59A              MOVE SECTOR DATA FROM DBUF0 TO STRING SPACE
LD51C          jsr       SYNCOMMA            SYNTAX CHECK FOR A COMMA
               ldx       #LB357              POINT X TO EVALUATE VARIABLE ROUTINE
               bsr       LD553               EVALUATE A VARIABLE
LD524          jmp       >LB146              'TM' ERROR IF NUMERIC VARIABLE
* EVALUATE DRIVE, TRACK AND SECTOR NUMBERS
LD527          jsr       EVALEXPB            EVALUATE EXPRESSION, RETURN VALUE IN ACCB
               jsr       >HITEST
               nop       
*	CMPB	#$03	* COMPARE TO 3 (HIGHEST DRIVE NUMBER) -
*	BHI	LD54A	* 'FC' ERROR IF IT’S > 3
               pshs      B                   SAVE DRIVE NUMBER ON THE STACK
               jsr       >LB738              SYNTAX CHECK FOR COMMA. EVALUATE EXPRESSION (TRACK NUMBER)
               cmpb      #TRKMAX-1           * CHECK FOR MAXIMUM TRACK NUMBER
               bhi       LD54A               * 'FC' ERROR IF TRACK NUMBER > 34
               pshs      B                   SAVE TRACK NUMBER ON THE STACK
               jsr       >LB738              SYNTAX CHECK FOR COMMA, EVALUATE EXPRESSION (SECTOR NUMBER)
               stb       DSEC                SAVE SECTOR NUMBER IN DSKCON VARIABLE
               decb                          *USELESS INSTRUCTION. NEXT INSTRUCTION SHOULD JUST
               cmpb      #SECMAX-1           *CHECK FOR MAXIMUM SECTOR NUMBER (SECMAX)
               bhi       LD54A               'FC' ERROR IF SECTOR NUMBER TOO BIG
               puls      A,B                 * GET TRACK AND DRIVE NUMBER OFF OF
               sta       DCTRK               * THE STACK AND SAVE IN DSKCON
               stb       DCDRV               * VARIABLES
               rts       
LD54A          jmp       >LB44A              JUMP TO 'FC' ERROR
LD54D          jsr       SYNCOMMA            SYNTAX CHECK FOR COMMA
               ldx       #LB156              POINT X TO 'EVALUATE EXPRESSION' ROUTINE ADDRESS
LD553          ldb       DCDRV               * GET THE DSKCON DRIVE, TRACK AND
               ldu       DCTRK               * SECTOR VALUES AND SAVE THEM ON THE STACK
               pshs      U,B                 *
               jsr       ,X                  GO EVALUATE AN EXPRESSION OR A VARIABLE
               puls      B,U                 * GET THE DRIVE, TRACK AND SECTOR
               stb       DCDRV               * NUMBERS OFF OF THE STACK AND PUT
               stu       DCTRK               * THEM BACK INTO THE DSKCON VARIABLES
               rts       
* DSKO$ COMMAND
DSKO           bsr       LD527               GET THE DRIVE, TRACK AND SECTOR NUMBERS
               bsr       LD54D               GET THE DESCRIPTOR OF STRING 1
               bsr       LD524               'TM' ERROR IF NUMERIC EXPRESSION
               ldx       FPA0+2              * GET STRING 1 DESCRIPTOR ADDRESS
               pshs      X                   * AND SAVE IT ON THE STACK
               bsr       LD54D               GET THE DESCRIPTOR OF STRING 2
               jsr       >LB654              *GET LENGTH AND ADDRESS OF STRING 2 AND
               pshs      X,B                 *SAVE THEM ON THE STACK
               clrb                          SET	CLEAR COUNTER TO 256 (FULL SECTOR BUFFER)
               ldx       #DBUF0              USE DBUF0 AS THE DSKO$ I/O BUFFER
LD577          clr       ,X+                 CLEAR A BYTE IN I/O BUFFER
               decb                          DECREMENT CLEAR COUNTER
               bne       LD577               BRANCH IF ALL 256 BYTES NOT CLEARED
               puls      B,X                 GET THE LENGTH AND ADDRESS OF STRING 2
               ldu       #DBUF0+128          POINT X TO STRING 2 DESTINATION
               bsr       LD519               MOVE STRING 2 DATA INTO DBUF0
               puls      X                   POINT X TO STRING 1 DESCRIPTOR
               jsr       >LB659              GET THE LENGTH AND ADDRESS OF STRING 1
               ldu       #DBUF0              POINT U TO STRING 1 DESTINATION
               bsr       LD519               MOVE STRING 1 DATA INTO DBUF0
               ldb       #$03                DSKCON WRITE OP CODE
LD58F          ldx       #DBUF0              POINT X TO I/O BUFFER (DBUF0)
               stx       DCBPT               *
               stb       DCOPC               * SAVE NEW DSKCON BUFFER POINTER AND OP CODE VARIABLES
               jmp       >LD6F2              GO WRITE OUT A SECTOR
* DSKINI COMMAND
DSKINI         lbeq      LA61F               BRANCH TO 'DN' ERROR IF NO DRIVE NUMBER SPECIFIED
               jmp       >DSKINI2
*	JSR	>LD256	CALCULATE DRIVE NUMBER
LD5A0          ldb       #$04                SKIP FACTOR DEFAULT VALUE
               jsr       GETCCH              GET CURRENT INPUT CHAR FROM BASiC
               beq       LD5B2               BRANCH IF END OF LINE
               jsr       >LB738              SYNTAX CHECK FOR COMMA AND EVALUATE EXPRESSION
               cmpb      #17                 MAX VALUE OF SKIP FACTOR = 16
               lbhs      LB44A               'ILLEGAL FUNCTION CALL' IF BAD SKIP FACTOR
               jsr       >LA5C7              SYNTAX ERROR IF MORE CHARACTERS ON THE LINE
LD5B2          pshs      B                   SAVE SKIP FACTOR ON THE STACK
               ldx       #DBUF1+SECMAX       POINT TO END OF LOGICAL SECTOR NUMBER STORAGE AREA
               ldb       #SECMAX             18 SECTORS PER TRACK
LD5B9          clr       ,-X                 CLEAR A BYTE IN THE BUFFER
               decb                          CLEARED ALL 18?
               bne       LD5B9               KEEP GOING IF NOT
               clra                          RESET	PHYSICAL SECTOR COUNTER
               bra       LD5CE               START WITH FIRST PHYSICAL SECTOR = 1
* CALCULATE LOGICAL SECTOR NUMBERS
LD5C1          addb      ,S                  ADD SKIP FACTOR TO LOGICAL SECTOR COUNTER
LD5C3          incb                          ADD ONE TO LOGICAL SECTOR COUNTER
LD5C4          subb      #SECMAX             SUBTRACT MAX NUMBER OF SECTORS
               bhs       LD5C4               BRANCH UNTIL 0 > ACCB >= -18
               addb      #SECMAX             ADD 18, NOW ACCB IS 0-17
               tst       B,X                 IS ANYTHING STORED HERE ALREADY?
               bne       LD5C3               YES - GET ANOTHER SECTOR
LD5CE          inca                          * INCREMENT PHYSICAL SECTOR NUMBER AND
               sta       B,X                 * SAVE IT IN THE RAM BUFFER
               cmpa      #SECMAX             FINISHED WITH ALL SECTORS?
               blo       LD5C1               NO - KEEP GOING
               leas      $01,S               REMOVE SKIP FACTOR FROM STACK
               leax      -256,s              Reserve stack space
               pshs      x                   Put top of RAM on stack
               ldx       <$1F                Get bottom of RAM
               leax      6280,x              Add in 1 track
               cmpx      ,s++                Enough room?
               lbhi      LAC44               No, ?OM ERROR
               jsr       >HCLOSE             Close all disk files
               jsr       L95AC               Reset SAM
               clr       <DCOPC              Rezero drive
*	LDX	#DFLBUF+$1888-2	GET TOP OF RAM USED BY DSKINI
*	CMPX	MEMSIZ	IS IT > CLEARED AREA?
*	LBHI	LAC44	'OUT OF MEMORY' ERROR IF > CLEARED AREA
*	JSR	DVEC7	CLOSE ALL FILES
*	COM	DRESFL	SET RESET FLAG TO $FF - THIS WILL CAUSE A DOS RESET
*	LDS	#DBUF1+SECLEN	SET STACK TO TOP OF DBUF1
*	JSR	>L95AC	RESET SAM TO DISPLAY PAGE ZERO AND ALPHA GRAPHICS
*	LDA	#$00	YOU COULD DELETE THIS INSTRUCTION AND CHANGE FOLLOWING STA TO CLR
*	STA	DCOPC	RESTORE HEAD TO TRACK ZERO DSKCON OP CODE
               clr       DCTRK               SET DSKCON TRACK VARIABLE TO TRACK ZERO
               jsr       >LD6F2              RESTORE HEAD TO TRACK ZERO
               clr       RDYTMR              RESET THE READY TIMER
               lda       #$C0                * FOC READ ADDRESS CODE
               sta       FDCREG              *
               jsr       >LD7D1              CHECK DRIVE READY - WAIT UNTIL READY
               beq       LD620               BRANCH IF DRIVES READY
               jmp       >LD688              ERROR IF DRIVES NOT READY
LD606          cmpa      #22                 = CHECK FOR TRACK 22 (PRECOMPENSATION)
               blo       LD612               = AND BRANCH IF < TRACK 22 - NO PRECOMP
               lda       DRGRAM              * GET THE RAM IMAGE OF DSKREG, 'OR'
               ora       #$10                * IN THE PRECOMPENSATION FLAG AND
               sta       DSKREG              * SEND IT TO DSKREG
LD612          lda       #$50                = GET STEP IN COMMAND
*LD612	LDA	#$53	= GET STEP IN COMMAND
               sta       FDCREG              = AND SEND IT TO THE 1793
               nop       
               lbsr      LCD01
*	EXG	A,A	* DELAY AFTER ISSUING COMMAND TO 1793
*	EXG	A,A	*
               jsr       >LD7D1              CHECK DRIVE READY
               bne       LD688               BRANCH IF NOT READY - ISSUE AN ERROR
LD620          jsr       >LD7F0              WAIT A WHILE
               bsr       LD691               BUILD A FORMATTED TRACK IN RAM
               ldy       #FDCREG+3           Y POINTS TO 1793 DATA REGISTER
               orcc      #$50                DISABLE INTERRUPTS
               ldx       #LD64F              * GET RETURN ADDRESS AND STORE
               stx       DNMIVC              * IT IN THE NON MASKABLE INTERRUPT VECTOR
               ldx       >$1F                POINT X TO THE FORMATTED TRACK RAM IMAGE
*	LDX	#DFLBUF	POINT X TO THE FORMATTED TRACK RAM IMAGE
               lda       FDCREG              RESET STATUS OF THE 1793
               lda       #$FF                * ENABLE THE NMI FLAG TO VECTOR
               sta       NMIFLG              * OUT OF AN I/O LOOP UPON AN NMI INTERRUPT
               ldb       #$F4                = GET WRITE TRACK COMMAND AND
               stb       FDCREG              = SEND TO 1793
               lda       DRGRAM              * GET THE DSKREG RAM IMAGE AND 'OR' IN THE
               ora       #$80                * FLAG WHICH WILL ENABLE THE 1793 TO HALT
               sta       DSKREG              * THE 6809. SEND RESULT TO DSKREG
LD649          ldb       ,X+                 = GET A BYTE FROM THE FORMATTED TRACK
               stb       ,Y                  = RAM IMAGE, SEND IT TO THE 1793 AND
               bra       LD649               = LOOP BACK TO GET ANOTHER BYTE
LD64F          lda       FDCREG              GET STATUS
               andcc     #$AF                ENABLE INTERRUPTS
               anda      #$44                * KEEP ONLY WRITE PROTECT & LOST DATA
               sta       DCSTA               * AND SAVE IT IN THE DSKCON STATUS BYTE
               bne       LD688               BRANCH IF ERROR
               inc       DCTRK               SKIP TO THE NEXT TRACK
               lda       DCTRK               GET THE TRACK NUMBER
               cmpa      #TRKMAX             WAS IT THE LAST TRACK
               bne       LD606               NO - KEEP GOING
* VERIFY THAT ALL SECTORS ARE READABLE
               lda       #$02                = GET THE DSKCON READ OP CODE
               sta       DCOPC               = AND SAVE IT IN THE DSKCON VARIABLE
               ldx       #DBUF0              * POINT THE DSKCON BUFFER POINTER
               stx       DCBPT               * TO DBUF0
               ldu       #DBUF1              POINT U TO THE LOGICAL SECTOR NUMBERS
               clra                          RESET	THE TRACK COUNTER TO ZERO
LD66F          sta       DCTRK               SET THE DSKCON TRACK VARIABLE
               clrb                          RESET	THE SECTOR COUNTER
LD672          lda       B,U                 GET THE PHYSICAL SECTOR NUMBER
               sta       DSEC                SAVE DSKCON SECTOR VARIABLE
               jsr       >LD6F2              READ A SECTOR
               incb                          * INCREMENT THE SECTOR COUNTER
               cmpb      #SECMAX             * AND COMPARE IT TO MAXIMUM SECTOR NUMBER
               blo       LD672               * AND KEEP LOOPING IF MORE SECTORS LEFT
               lda       DCTRK               = GET THE CURRENT TRACK NUMBER
               inca                          = ADD ONE TO IT, COMPARE TO THE MAXIMUM TRACK
               cmpa      #TRKMAX             = NUMBER AND KEEP LOOPING IF
               blo       LD66F               = THERE ARE STILL TRACKS TO DO
               jmp       >BEEP
*	JMP	>LD2CD	GO CHECK FOR A DOS RESET
LD688          clr       DRGRAM              CLEAR RAM IMAGE OF DSKREG
               clr       DSKREG              CLEAR DSKREG - TURN DISK MOTORS OFF
               jmp       >LD701              PROCESS DRIVES NOT READY ERROR
* BUILD A FORMATTED TRACK OF DATA IN RAM STARTING AT DFLBUF.
LD691          ldx       >$1F
*LD691	LDX	#DFLBUF	START TRACK BUFFER AT DFLBUF
               ldd       #$204E              GET SET TO WRITE 32 BYTES OF $4E
               bsr       LD6C2               GO WRITE GAP IV
               clrb                          RESET	SECTOR COUNTER
LD69A          pshs      B                   SAVE SECTOR COUNTER
               ldu       #DBUF1              POINT U TO THE TABLE OF LOGICAL SECTORS
               ldb       B,U                 * GET LOGICAL SECTOR NUMBER FROM TABLE AND
               stb       DSEC                * SAVE IT IN THE DSKCON VARIABLE
               ldu       #LD6D4              POINT U TO TABLE OF SECTOR FORMATTING DATA
               ldb       #$03                * GET FIRST 3 DATA BLOCKS AND
               bsr       LD6C8               * WRITE THEM TO BUFFER
               lda       DCTRK               = GET TRACK NUMBER AND STORE lT
               sta       ,X+                 = IN THE RAM BUFFER
               lbsr      TFSIDE              branch to sub routine that deals with the side
               NOP                           This NOP is used to keep rom spacing correct
*               clr       ,X+                 CLEAR A BYTE (SIDE NUMBER) IN BUFFER
*               lda       DSEC                * GET SECTOR NUMBER AND
               sta       ,X+                 * STORE IT IN THE BUFFER
               ldb       #$09                = GET THE LAST NINE DATA BLOCKS AND
               bsr       LD6C8               = WRITE THEM TO THE BUFFER
               puls      B                   GET SECTOR COUNTER
               incb                          NEXT SECTOR
               cmpb      #SECMAX             18 SECTORS PER TRACK
               blo       LD69A               BRANCH IF ALL SECTORS NOT DONE
               ldd       #$C84E              WRITE 200 BYTES OF $4E AT END OF TRACK
* WRITE ACCA BYTES OF ACCB INTO BUFFER
LD6C2          stb       ,X+                 STORE A BYTE IN THE BUFFER
               deca                          DECREMENT COUNTER
               bne       LD6C2               BRANCH IF ALL BYTES NOT MOVED
               rts       
LD6C8          pshs      B                   SAVE THE COUNTER ON THE STACK
               ldd       ,U++                GET TWO BYTES OF DATA FROM THE TABLE
               bsr       LD6C2               WRITE ACCA BYTES OF ACCB INTO THE BUFFER
               puls      B                   * GET THE COUNTER BACK, DECREMENT
               decb                          * IT AND BRANCH IF ALL DATA BLOCKS
               bne       LD6C8               * NOT DONE
               rts       
* DATA USED TO FORMAT A SECTOR ON THE DISK
* THESE DATA ARE CLOSE TO THE IBM SYSTEM 34 FORMAT FOR 256 BYTE SECTORS.
* DOUBLE DENSITY. THE FORMAT GENERALLY CONFORMS TO THAT SPECIFIED ON THE
* 1793 DATA SHEET. THE GAP SIZES HAVE BEEN REDUCED TO THE MINIMUM
* ALLOWABLE. THE IBM FORMAT USES $40 AS THE FILL CHARACTER FOR THE DATA
* BLOCKS WHILE COLOR DOS USES AN $FF AS THE FILL CHARACTER.
LD6D4          fcb       8,0                 SYNC FIELD
               fcb       3,$F5
               fcb       1,$FE               ID ADDRESS MARK (AM1)
* TRACK, SIDE, AND SECTOR NUMBERS ARE INSERTED HERE
               fcb       1,1                 SECTOR SIZE (256 BYTE SECTORS)
               fcb       1,$F7               CRC REQUEST
               fcb       22,$4E              GAP II (POST-ID GAP)
               fcb       12,0                SYNC FIELD
               fcb       3,$F5
               fcb       1,$FB               DATA ADDRESS MARK (AM2)
               fcb       0,$FF               DATA FIELD (256 BYTES)
               fcb       1,$F7               CRC REQUEST
               fcb       24,$4E              GAP III (POST DATA GAP)
* DOS COMMAND
DOS            nop       
               nop       
*DOS	BNE	LD742	RETURN IF ARGUMENT GIVEN
               jmp       [DOSVEC]            JUMP TO THE DOS COMMAND
LD6F2          pshs      B                   SAVE ACCB
               ldb       #$05                5 RETRIES
               stb       ATTCTR              SAVE RETRY COUNT
               puls      B                   RESTORE ACCB
LD6FB          bsr       DSKCON              GO EXECUTE COMMAND
               tst       DCSTA               CHECK STATUS
               beq       LD70E               BRANCH IF NO ERRORS
LD701          lda       DCSTA               GET DSKCON ERROR STATUS
               ldb       #2*30               'WRITE PROTECTED' ERROR
               bita      #$40                CHECK BIT 6 OF STATUS
               bne       LD70B               BRANCH IF WRITE PROTECT ERROR
LD709          ldb       #2*20               'I/O ERROR'
LD70B          jmp       >LAC46              JUMP TO ERROR DRIVER
LD70E          pshs      A                   SAVE ACCA
               lda       DCOPC               GET OPERATION CODE
               cmpa      #$03                CHECK FOR WRITE SECTOR COMMAND
               puls      A                   RESTORE ACCA
               bne       LD742               RETURN IF NOT WRITE SECTOR
               tst       DVERFL              CHECK VERIFY FLAG
               beq       LD742               RETURN IF NO VERIFY
               pshs      U,X,B,A             SAVE REGISTERS
               lda       #$02                READ OPERATION CODE
               sta       DCOPC               STORE TO DSKCON PARAMETER
               ldu       DCBPT               POINT U TO WRITE BUFFER ADDRESS
               ldx       #DBUF1              * ADDRESS OF VERIFY BUFFER
               stx       DCBPT               * TO DSKCON VARIABLE
               bsr       DSKCON              GO READ SECTOR
               stu       DCBPT               RESTORE WRITE BUFFER
               lda       #$03                WRITE OP CODE
               sta       DCOPC               SAVE IN DSKCON VARIABLE
               lda       DCSTA               CHECK STATUS FOR THE READ OPERATION
               bne       LD743               BRANCH IF ERROR
               clrb                          CHECK	256 BYTES
LD737          lda       ,X+                 GET BYTE FROM WRITE BUFFER
               cmpa      ,U+                 COMPARE TO READ BUFFER
               bne       LD743               BRANCH IF NOT EQUAL
               decb                          * DECREMENT BYTE COUNTER AND
               bne       LD737               * BRANCH IF NOT DONE
               puls      A,B,X,U             RESTORE REGISTERS
LD742          rts       
LD743          puls      A,B,X,U             RESTORE REGISTERS
               dec       ATTCTR              DECREMENT THE VERIFY COUNTER
               bne       LD6FB               BRANCH IF MORE TRIES LEFT
               ldb       #2*36               'VERIFY ERROR'
               bra       LD70B               JUMP TO ERROR HANDLER
* VERIFY COMMAND
VERIFY         clrb                          OFF FLAG = 0
               cmpa      #TOKEN_OFF          OFF TOKEN ?
               beq       LD75A               YES
               comb                          ON FLAG = $FF
               cmpa      #TOKEN_ON           ON TOKEN ?
               lbne      LB277               BRANCH TO 'SYNTAX ERROR' IF NOT ON OR OFF
LD75A          stb       DVERFL              SET VERIFY FLAG
               jmp       GETNCH              GET NEXT CHARACTER FROM BASIC
* DSKCON ROUTINE
* Re-Route DSKCON
DSKCON         jmp       >DSKCON2
               nop       
*DSKCON	PSHS	U,Y,X,B,A	SAVE REGISTERS
*	LDA	#$05	* GET RETRY COUNT AND
LD763          pshs      A                   * SAVE IT ON THE STACK
LD765          clr       RDYTMR              RESET DRIVE NOT READY TIMER
               ldb       DCDRV               GET DRIVE NUMBER
               ldx       #LD89D              POINT X TO DRIVE ENABLE MASKS
               lda       DRGRAM              GET DSKREG IMAGE
               anda      #$A8                KEEP MOTOR STATUS, DOUBLE DENSITY. HALT ENABLE
               ora       B,X                 'OR' IN DRIVE SELECT DATA
               ora       #$20                'OR' IN DOUBLE DENSITY
               ldb       DCTRK               GET TRACK NUMBER
               cmpb      #22                 PRECOMPENSATION STARTS AT TRACK 22
               blo       LD77E               BRANCH IF LESS THAN 22
               ora       #$10                TURN ON WRITE PRECOMPENSATION IF >= 22
LD77E          tfr       A,B                 SAVE PARTIAL IMAGE IN ACCB
               ora       #$08                'OR' IN MOTOR ON CONTROL BIT
               sta       DRGRAM              SAVE IMAGE IN RAM
               sta       DSKREG              PROGRAM THE 1793 CONTROL REGISTER
               bitb      #$08                = WERE MOTORS ALREADY ON?
               bne       LD792               = DON'T WAIT FOR IT TO COME UP TO SPEED IF ALREADY ON
* Floppy Motor Rev-Up Delay
               fcb       SKIP2
               fdb       LA7D1
*	JSR	>LA7D1	* WAIT A WHILE
               jsr       >LA7D1              * WAIT SOME MORE FOR MOTOR TO COME UP TO SPEED
LD792          bsr       LD7D1               WAIT UNTIL NOT BUSY OR TIME OUT
               bne       LD7A0               BRANCH IF TIMED OUT (DOOR OPEN. NO DISK, NO POWER. ETC.)
               clr       DCSTA               CLEAR STATUS REGISTER
               ldx       #LD895              POINT TO COMMAND JUMP VECTORS
               ldb       DCOPC               GET COMMAND
               aslb                          2 BYTES PER COMMAND JUMP ADDRESS
               jsr       [B,X]               GO DO IT
LD7A0          puls      A                   GET RETRY COUNT
               ldb       DCSTA               GET STATUS
               beq       LD7B1               BRANCH IF NO ERRORS
               deca                          DECREMENT RETRIES COUNTER
               beq       LD7B1               BRANCH IF NO RETRIES LEFT
               pshs      A                   SAVE RETRY COUNT ON STACK
               bsr       LD7B8               RESTORE HEAD TO TRACK 0
               bne       LD7A0               BRANCH IF SEEK ERROR
               bra       LD765               GO TRY COMMAND AGAIN IF NO ERROR
* Floppy drive motors off 4 seconds
LD7B1          lda       #(4*60)
*LD7B1	LDA	#120	120*1/60 = 2 SECONDS (1/60 SECOND FOR EACH IRQ INTERRUPT)
               sta       RDYTMR              WAIT 2 SECONDS BEFORE TURNING OFF MOTOR
               puls      A,B,X,Y,U,PC        RESTORE REGISTERS - EXIT DSKCON
* RESTORE HEAD TO TRACK 0
LD7B8          jsr       >TRKTBL
               nop       
               nop       
*LD7B8	LDX	#DR0TRK	POINT TO TRACK TABLE
*	LDB	DCDRV	GET DRIVE NUMBER
               clr       B,X                 ZERO TRACK NUMBER
* Restore Step Rate
               lda       #$00                * RESTORE HEAD TO TRACK 0, UNLOAD THE HEAD
*	LDA	#$03	* RESTORE HEAD TO TRACK 0, UNLOAD THE HEAD
               sta       FDCREG              * AT START, 30 MS STEPPING RATE
               nop       
               lbsr      LCD01
*	EXG	A,A	=
*	EXG	A,A	= WAIT FOR 1793 TO RESPOND TO COMMAND
               bsr       LD7D1               WAIT TILL DRIVE NOT BUSY
               bsr       LD7F0               WAIT SOME MORE
               anda      #$10                1793 STATUS : KEEP ONLY SEEK ERROR
               sta       DCSTA               SAVE IN DSKCON STATUS
LD7D0          rts       
* WAIT FOR THE 1793 TO BECOME UNBUSY. IF IT DOES NOT BECOME UNBUSY,
* FORCE AN INTERRUPT AND ISSUE A ‘DRIVE NOT READY’ 1793 ERROR.
LD7D1          ldx       ZERO                GET ZERO TO X REGISTER - LONG WAIT
LD7D3          leax      -1,X                DECREMENT LONG WAIT COUNTER
               beq       LD7DF               lF NOT READY BY NOW, FORCE INTERRUPT
               lbsr      LC101               Delay, then LDA $FF48
*	LDA	FDCREG	* GET 1793 STATUS AND TEST
               bita      #$01                * BUSY STATUS BIT
               bne       LD7D3               BRANCH IF BUSY
               rts       
LD7DF          lda       #$D0                * FORCE INTERRUPT COMMAND - TERMINATE ANY COMMAND
               sta       FDCREG              * IN PROCESS. DO NOT GENERATE A 1793 INTERRUPT REQUEST
               nop       
               lbsr      LCD01
*	EXG	A,A	* WAIT BEFORE READING 1793
*	EXG	A,A	*
               lda       FDCREG              RESET INTRQ (FDC INTERRUPT REQUEST)
               lda       #$80                RETURN DRIVE NOT READY STATUS IF THE DRIVE DID NOT BECOME UNBUSY
               sta       DCSTA               SAVE DSKCON STATUS BYTE
               rts       
* MEDIUM DELAY
LD7F0          ldx       #8750               DELAY FOR A WHILE
LD7F3          leax      -1,X                * DECREMENT DELAY COUNTER AND
               bne       LD7F3               * BRANCH IF NOT DONE
               rts       
* READ ONE SECTOR
LD7F8          lda       #$80                $80 IS READ FLAG (1793 READ SECTOR)
LD7FA          fcb       $8C                 SKIP TWO BYTES (THROWN AWAY CMPX INSTRUCTION)
* WRITE ONE SECTOR
LD7FB          lda       #$A0                $A0 IS WRITE FLAG (1793 WRITE SECTOR)
               pshs      A                   SAVE READ/WRITE FLAG ON STACK
* Allow more time before FDC not ready
               jsr       >TRKTBL
               nop       
               nop       
*	LDX	#DR0TRK	POINT X TO TRACK NUMBER TABLE IN RAM
*	LDB	DCDRV	GET DRIVE NUMBER
               abx                           POINT X TO CORRECT DRIVE'S TRACK BYTE
               ldb       ,X                  GET TRACK NUMBER OF CURRENT HEAD POSITION
               stb       FDCREG+1            SEND TO 1793 TRACK REGISTER
               cmpb      DCTRK               COMPARE TO DESIRED TRACK
               beq       LD82C               BRANCH IF ON CORRECT TRACK
               lda       DCTRK               GET TRACK DESIRED
               sta       FDCREG+3            SEND TO 1793 DATA REGiSTER
               sta       ,X                  SAVE IN RAM TRACK IMAGE
* Seek Step Rate
               lda       #$14                * SEEK COMMAND FOR 1793: DO NOT LOAD THE
*	LDA	#$17	* SEEK COMMAND FOR 1793: DO NOT LOAD THE
               sta       FDCREG              * HEAD AT START, VERIFY DESTINATION TRACK,
               nop       
               lbsr      LCD01
*	EXG	A,A	* 30 MS STEPPING RATE - WAIT FOR
*	EXG	A,A	* VALID STATUS FROM 1793
               bsr       LD7D1               WAIT TILL NOT BUSY
               bne       LD82A               RETURN IF TIMED OUT
               bsr       LD7F0               WAIT SOME MORE
               anda      #$18                KEEP ONLY SEEK ERROR OR CRC ERROR IN ID FIELD
               beq       LD82C               BRANCH IF NO ERRORS - HEAD ON CORRECT TRACK
               sta       DCSTA               SAVE IN DSKCON STATUS
LD82A          puls      A,PC
* HEAD POSITIONED ON CORRECT TRACK
LD82C          lda       DSEC                GET SECTOR NUMBER DESIRED
               sta       FDCREG+2            SEND TO 1793 SECTOR REGISTER
               ldx       #LD88B              * POINT X TO ROUTINE TO BE VECTORED
               stx       DNMIVC              * TO BY NMI UPON COMPLETION OF DISK I/O AND SAVE VECTOR
               ldx       DCBPT               POINT X TO I/O BUFFER
               lda       FDCREG              RESET INTRQ (FDC INTERRUPT REQUEST)
               lda       DRGRAM              GET DSKREG IMAGE
               ora       #$80                SET FLAG TO ENABLE 1793 TO HALT 6809
               puls      B                   GET READ/WRITE COMMAND FROM STACK
               ldy       ZERO                ZERO OUT Y - TIMEOUT INITIAL VALUE
               ldu       #FDCREG             U POINTS TO 1793 INTERFACE REGISTERS
               com       NMIFLG              NMI FLAG = $FF: ENABLE NMI VECTOR
               orcc      #$50                DISABLE FIRQ,IRQ
               stb       FDCREG              * SEND READ/WRITE COMMAND TO 1793: SINGLE RECORD, COMPARE
               nop       
               lbsr      LCD01
*	EXG	A,A	* FOR SIDE 0, NO 15 MS DELAY, DISABLE SIDE SELECT
*	EXG	A,A	* COMPARE, WRITE DATA ADDRESS MARK (FB) - WAIT FOR STATUS
               cmpb      #$80                WAS THIS A READ?
               beq       LD875               IF SO, GO LOOK FOR DATA
* WAIT FOR THE 1793 TO ACKNOWLEDGE READY TO WRITE DATA
               ldb       #$02                DRQ MASK BIT
LD85B          bitb      ,U                  IS 1793 READY FOR A BYTE? (DRQ SET IN STATUS BYTE)
               bne       LD86B               BRANCH IF SO
               leay      -1,Y                DECREMENT WAIT TIMER
               bne       LD85B               KEEP WAITING FOR THE 1793 DRQ
LD863          clr       NMIFLG              RESET NMI FLAG
               andcc     #$AF                ENABLE FIRQ,IRQ
               jmp       >LD7DF              FORCE INTERRUPT, SET DRIVE NOT READY ERROR
* WRITE A SECTOR
LD86B          ldb       ,X+                 GET A BYTE FROM RAM
               stb       FDCREG+3            SEND IT TO 1793 DATA REGISTER
               sta       DSKREG              REPROGRAM FDC CONTROL REGISTER
               bra       LD86B               SEND MORE DATA
* WAIT FOR THE 17933 TO ACKNOWLEDGE READY TO READ DATA
LD875          ldb       #$02                DRQ MASK BIT
LD877          bitb      ,U                  DOES THE 1793 HAVE A BYTE? (DRQ SET IN STATUS BYTE)
               bne       LD881               YES, GO READ A SECTOR
               leay      -1,Y                DECREMENT WAIT TIMER
               bne       LD877               KEEP WAITING FOR 1793 DRQ
               bra       LD863               GENERATE DRIVE NOT READY ERROR
* READ A SECTOR
LD881          ldb       FDCREG+3            GET DATA BYTE FROM 1793 DATA REGISTER
               stb       ,X+                 PUT IT IN RAM
               sta       DSKREG              REPROGRAM FDC CONTROL REGISTER
               bra       LD881               KEEP GETTING DATA
* BRANCH HERE ON COMPLETION OF SECTOR READ/WRITE
LD88B          andcc     #$AF                ENABLE IRQ, FIRO
               lda       FDCREG              * GET STATUS & KEEP WRITE PROTECT, RECORD TYPE/WRITE
               anda      #$7C                * FAULT, RECORD NOT FOUND, CRC ERROR OR LOST DATA
               sta       DCSTA               SAVE IN DSKCON STATUS
               rts       
* DSKCON OPERATION CODE JUMP VECTORS
LD895          fdb       LD7B8               RESTORE HEAD TO TRACK ZERO
               fdb       LD7D0               NO OP - RETURN
               fdb       LD7F8               READ SECTOR
               fdb       LD7FB               WRITE SECTOR
* DSKREG MASKS FOR DISK DRIVE SELECT
LD89D          fcb       1                   DRIVE SEL 0
               fcb       2                   DRIVE SEL 1
               fcb       $40+1               DRIVE SEL 2 (back of 0)
               fcb       $40+2               DRIVE SEL 3 (back of 1)
*	FCB	4	DRIVE SEL 2
*	FCB	$40	DRIVE SEL 3
* NMI SERVICE
DNMISV         lda       NMIFLG              GET NMI FLAG
               beq       LD8AE               RETURN IF NOT ACTIVE
               ldx       DNMIVC              GET NEW RETURN VECTOR
               stx       10,S                STORE AT STACKED PC SLOT ON STACK
               clr       NMIFLG              RESET NMI FLAG
LD8AE          rti       
* IRQ SERVICE
DIRQSV         lda       PIA0+3              63.5 MICRO SECOND OR 60 HZ INTERRUPT?
               bpl       LD8AE               RETURN IF 63.5 MICROSECOND
               lda       PIA0+2              RESET 60 HZ PIA INTERRUPT FLAG
               lda       RDYTMR              GET TIMER
               beq       LD8CD               BRANCH IF NOT ACTIVE
               deca                          DECREMENT THE TIMER
               sta       RDYTMR              SAVE IT
               bne       LD8CD               BRANCH IF NOT TIME TO TURN OFF DISK MOTORS
               lda       DRGRAM              = GET DSKREG IMAGE
               anda      #$B0                = TURN ALL MOTORS AND DRIVE SELECTS OFF
               sta       DRGRAM              = PUT IT BACK IN RAM IMAGE
               sta       DSKREG              SEND TO CONTROL REGISTER (MOTORS OFF)
LD8CD          jmp       >L8955              JUMP TO EXTENDED BASIC'S IRQ HANDLER
* THIS IS THE END OF DISK BASIC (EXCEPT FOR THE DOS COMMAND AT $DF00).
* THE CODE FROM THIS POINT TO $DF00 IS GARBAGE.
* DOSBAS 1.1 = 1584 WASTED BYTES
** THIS IS THE CODE FOR THE DOS COMMAND
DOSCOM         swi3                          user hook
               ldd       #$200               Read, drive 0
               std       <DCOPC              Tell DSKCON
               jsr       >LD252               DOS (drive number)?
               jsr       LA5C7               ?SN ERROR if more chars
               ldd       #(34*256)+1         track 34, sector 1
               std       <DCTRK              Tell DSKCON
               ldd       #DBUF0              DOS buffer
               std       <DCBPT              Tell DSKCON
               jsr       >LD6F2               Read disk into buffer
               ldd       DBUF0               Get first 2 bytes
               cmpd      #('O*256)+'S        "OS" header?
               lbne      DOAUTO              No, try AUTOEXEC
a@             ldd       #DOSBUF
               std       <DCBPT
               ldb       #18
b@             jsr       >LD6F2
               inc       <DCSEC              Sector
               inc       <DCBPT              Pointer
               decb                          Proceed through track
               bne       b@                  Loop
               jmp       $2602               Execute the program

* DOS INITALIZATION
DOSINI         pshs      d,x                 Save registers
DOSIN2         ldd       #(6*256)+$3B        6 "RTI" opcodes
               ldx       #$100               Point to SWI vectors
               jsr       >LD6C2               Store 6 RTIs
               puls      d,x,pc              Restore & return

* SIDE select portion of the track buffer format routine.
TFSIDE         pshs      x                   Backup X onto stack
               ldx       #LD89D              Set X to point to drive table
               ldb       <DCDRV              Get current drive from DCDRV
               ldb       b,x                 Get drive masks from drive table for B
               puls      x                   Restore X from stack
               clra                          clear A to have it ready for bit6 from B
               lslb                          roll bit 7 into C
               lslb                          roll bit 6 into N
               rola                          roll bit 6 back from N into A
               sta       ,x+                 store A into track format buffer
               lda       DSEC                * GET SECTOR NUMBER AND
               rts
* These fcb's are filler
               fcb       $FF,$FF,$FF,$FF,$FF
* This fcb is also filler and is used to help check ROM locations
               fcb       $99



* HDB-DOS Version
VMAJOR         equ       1
VMINOR         equ       4
VREV           equ       0


               setdp     0


STOP2          equ       0

* FlexiKey and Directory Equates
GETKEY         equ       LA1B1
HLDPTR         equ       $1D1
INSERT         equ       $1D2
WHLINE         equ       $1D3
DIR1           equ       $1D4
DIR2           equ       $1D5
HLDBFR         equ       $1DA
BASBFR         equ       $2DD


* Static Storage                             (Reusing 9 last bytes of original USR table, after stubs)
               IFDEF     DRAGON
               org       $13F
               ELSE
               org       $149
               ENDC

INTFLG         rmb       1                   FlexiKey variable
NCYLS          rmb       2                   Device cylinder count (IDE)
NHEADS         rmb       1                   Device head count (IDE)
NSECTS         rmb       1                   Device sector count (IDE)
HDFLAG         rmb       1                   Hard drive active flag
DRVSEL         rmb       1                   LUN (SCSI), Master/Slave (IDE) or Drive Number (DW)
RETRY          equ       DRVSEL              DriveWire uses this location as a retry counter
MAXDRV         rmb       1                   Highest drive number
IDNUM          rmb       1                   Device number (SCSI 0-7) (IDE 0-1)


* Dynamic Storage
               org       $F3

VCMD           rmb       1                   SCSI/IDE unit command
VAD0           rmb       1                   L.U.N. / sector (hi-byte)
VAD1           rmb       2                   Sector (lo-word)
VBLKS          rmb       2                   Block count / options
VEXT           rmb       4                   Reserved 10 byte SCSI commands


* HARD DISK DRIVER

               org       MAGICDG+$1930       It all starts here!

* Indirect Jump Table ( jsr [$MMMM] )

               fdb       DISKIO              universal hard disk input / output
               fdb       SETUP               Setup command packet
               fdb       BEEP                Make a beep sound
               fdb       DSKCON2             DSKCON Re-entry

HDBHI          fcb       $00                 HDB-DOS Offset hi-byte
HDBLO          fdb       $0000               HDB-DOS Offset lo-word

PORT           fdb       DATAADDR            Interface base address
CCODE          fcb       TDELAY              IDE: startup delay / SCSI: wakeup delay
DEFID          fcb       0

               IFDEF     DW
               fdb       DWRead
               fdb       DWWrite
               ENDC

****************************************
* OBJECT CODE STARTS HERE
****************************************

HDINIT         ldx       #SIGNON-1           Point to sign-on
               lbsr      PRINT2              Print it

               ldd       #(9*256)+0          Write 9 bytes of 0...
               ldx       #INTFLG             ...Into our RAM (clear it)
               jsr       >LD6C2               Erase our RAM; write regA bytes of regB data to regX
               ldx       #LA0E2              Warm start BASIC
               pshs      x                   Save off for later RTS

               ldd       #$FFFF              "DIRECT" line number
               std       <$68                Set BASIC "DIRECT" mode
               IFDEF     IDE
*         ldd   #$0101    
               std       NHEADS              To prevent CHS routine product from being 0!
               ENDC      

               ldd       #$5503              Warm start flag & drive # R.G.
               sta       <$71                Enable warm start
               stb       MAXDRV              Set Max Drive

               IFNE      DW-1
               ldu       PORT                Get port address
               IFNE      IDE+TC3-1
               clr       SCSIRESET,u         Reset controller
               ENDC      

* Here we attempt to detect the presense of the controller or
* device by writing to a verification address and reading back
* to see if the read matched the write.

               IFDEF     IDE
* Default ID is always MASTER
               clr       IDNUM               save MASTER as ID number

* IDE Interface Initialization
*
* We send the EXECUTE DEVICE DIAGNOSTIC command
* to the device, then look for the master having passed.
* FIX: PQI 256MB CF needs the following two lines.  Also not
* a bad idea for *ANY* IDE device.
               lbsr      W4NBUSY
               bne       FAIL

               lda       #EXECDIAG
               sta       CMD,u
*	ldb	CCODE
*CnTest	tst	STATUS,u
*	bpl	CnChk
*	decb		Else decrement retry counter
*	bne	CnTest	If not zero, retry test
               lbsr      W4NBUSY
               bne       FAIL
CnChk          lda       ERR,u
               cmpa      #$01                device 0 passed?
               bne       FAIL
               ENDC      

               IFDEF     SCSI
* Set up default ID
               ldb       DEFID               get default ID
               stb       IDNUM               save it as ID number

* SCSI Controller Initialization
*
* SCSI controllers are usually available as soon as power
* is applied, so we do not loop, we just check once to see
* if they are available.
               jsr       >LD7F0               Reset delay
               sta       DATARW,u            Write to port to verify controller exists
               cmpa      DATARW,u            Is interface there?
               bne       FAIL                No, set up for floppy
               clr       DATARW,u            Else clear out verification

* This loop waits until the drive has successfully restored
* itself.  It is a locked loop, and if a SCSI controller is
* available but no drives are on it, this will loop forever.
a@             jsr       >REZERO             Reset drive to 00
               tst       <DCSTAT             Any errors?
               bne       a@

               ENDC      

* If we are here, the hard disk is on-line and ready
CtlrOk         lbsr      GETMAX              Compute maximum no. of drives
               tst       <DCSTAT             errors?
               bne       FAIL
               ENDC      

* Initialize DriveWire
               IFDEF     DW
               lbsr      GETMAX              Compute maximum no. of drives

* Turbo Mode for DW4
               IFDEF     DW4
               lda       #$E6                turbo notification command
               sta       VCMD
               ldy       #1
               jsr       >SENDY              tell server we want turbo mode
               ldx       #$8000              delay counter
               jsr       >LD7F3               delay while server changes baud rate
               ENDC      

*               pshs      cc                  then push CC on stack
*               orcc      #IntMasks
*               ldx       #PIA1Base           $FF20
*               clr       1,x                 clear CD
*               lda       #$FE
*               sta       ,x
*               lda       #$36
*               sta       1,x
*               lda       ,x
* Send INIT to the server
*               lda       #OP_INIT
*               IFGT      Level-1
*               clr       $FFD9
*               ENDC      
*               pshs      a
*               leax      ,s
*               ldy       #0001
*               lbsr      DWWrite
*               puls      a,cc                Carry is clear on pull
               ENDC      

               IFDEF     ARDUINO
* setup PIA PORTA (read)
               clr       $FF51
               clr       $FF50
               lda       #$2C
               sta       $FF51

* setup PIA PORTB (write)
               clr       $FF53
               lda       #$FF
               sta       $FF52
               lda       #$2C
               sta       $FF53
               ENDC

ITSOK          jsr       >BEEP               Send a beep
               IFNE      DW-1
               bra       BOOTUP              Try AUTOEXEC

* From this point floppies are enabled
FAIL           ldx       #FAILED-1           Point to message
FAIL2          clr       <DCDRV              Insure drive 0 boot
               bsr       PRINT2
               ldb       #$04
               jsr       >DCLOSE
               ENDC      

* Test for SHIFT key, skip autoboot if pressed
BOOTUP         lda       #$7F                Shift bit mask
* Optimizations below save 1 byte
*         ldx   #$FF00     Point to keyboard PIA
*         sta   2,x        Set columns
               sta       $FF02               Set columns
*         lda   ,x         Read rows
               lda       $FF00               Read rows
               coma                          Invert
               anda      #$40                Keep SHIFT
               bne       BASICBYE            Was pressed, go to BASIC

* No SHIFT key, see if the drive is working
               ldb       #6                  Move 6 bytes
               ldx       #DTEST              Read, D=0, T=0, S=1, B=$600
               ldu       DSKVAR              Point to DSKCON variables
               jsr       LA59A               Move (B) From X to U
               ldb       <DCDRV              Get current drive
               stb       <DCDRV              Store for DSKCON
               jsr       [DCNVEC]            Try to read disk
               tst       <DCSTAT             Did we get an I/O error?
               bne       BASICBYE            Yes, just go to BASIC

* Try to find & run the AUTOEXEC.BAS file
* If file is MACHINE CODE, do not try to run it!
DOAUTO         ldx       #NAMBUF             Point to "AUTOEXEC.BAS"
               jsr       >LD3B1               Get it into diskname buffer
               jsr       >LC68C               See if it exists
               tst       $973                Is it there? Existing file sector#
               beq       BASICBYE            No, goto BASIC
               ldx       $974                Yes, point to dir data
               ldd       11,x                Get filetype / ASCII flag
               subd      #$200               Machine code, binary?
               beq       BASICBYE
               lda       <DCDRV              Get DSKCON drive #
               sta       $95A                And make it default drive for BASIC
               leas      2,s
               jmp       LCA67               No, go run it!
BASICBYE       rts       


* This is DSKINI for the hard drive
DSKINI2        jsr       >LD256               Evaluate drive number
               cmpb      HDFLAG              Is it a hard disk?
               lbcs      LD5A0               No, goto old code

               jsr       LA5C7               ?SN ERROR if more chars
               jsr       >HCLOSE             Close all disk files

               lda       <$68                Get line number MSB
               inca                          In direct mode?
               bne       ERASE2              No, skip prompting

               ldx       #FORMT-1            "FORMAT HARD DISK"
               bsr       PRINT2              Print it
               clra                          Zero top of D
               ldb       <DCDRV              Get drive number
               jsr       LBDCC               Print it
               ldx       #RDYMSG-1           "ARE YOU SURE? (Y/N)"
               bsr       PRINT2              Print it
               jsr       >GETY               Go look for the "Y" key
               beq       ERASE               Yes, go erase hard disk
               ldx       #ABTMSG-1           Not Y, aborted
PRINT2         jmp       STRINOUT            Print it and return


* This erases all sectors of a hard disk with $FFs
* R.G. One less byte used with the next command more clock cycles used
ERASE          jsr       >LCCFD               However, $CCFD is jmp [$A002]
ERASE2         lda       #3                  DSKCON Write opcode
               sta       <DCOPC              Tell DSKCON
               ldd       #(34*256)+18        Begin at track 34, sector 18
               std       <DCTRK              Tell DSKCON
               ldd       #(0*256)+$FF        Buffer fill 256 bytes of $FF
               ldx       #DBUF0              Point to buffer
               stx       <DCBPT              Tell DSKCON where it is
               jsr       >LD6C2               Fill buffer with $FF'S
a@             jsr       >LD6F2               Write & verify a sector of $FF'S
               dec       <DCSEC              Decrement sector count
               bne       a@                  Go until sector = 0
               lda       #18                 Reset sector variable...
               sta       <DCSEC              ...Back to 18 for next track
               dec       <DCTRK              Decrement track count
               bpl       a@                  Go until track = -1

               jmp       >BEEP               Signal done


* DSKCON goes here for floppy drives
DSK20          lda       #3                  Original..... (WAS lda #5)
               jmp       LD763               DSKCON code

* DSKCON now enters here
DSKCON2        pshs      d,x,y,u             Save registers
               lda       <DCDRV              Get drive number desired
               cmpa      HDFLAG              Is it a hard disk?
               bcs       DSK20               No, do floppies

* Calculate sector addresses for track & sector
               ldy       #VCMD               Y points to command packet for temp storage
               ldu       #HDBHI              U points to 3 byte HDB-DOS offset value
               sta       ,y                  Save off drive number here
               clr       1,y
               lda       <DCTRK              Get track number
               ldb       #18                 18 sectors per track
               mul                           Multiply
               addb      <DCSEC              Add in sectors
               adca      #0                  Propagate carry
               subd      #1                  Normalize sectors to 0-17

* Add in drives offset
               tst       ,y                  Last drive?
               beq       c@                  Yes, exit this loop
a@             addd      #(18*35)            No, add in 1 drive
               bcc       b@                  skip inc if < $FFFF
               inc       1,y                 Bump hi-byte if > $FFFF 
b@             dec       ,y                  Count down drive
               bne       a@                  Go again
c@             addd      1,u

* Add in HDB-DOS offset

               bcc       a@                  Go if no overflow
               inc       1,y
a@             tfr       d,x                 Put lo-word in X
               ldb       1,y                 Get sector address hi-byte
               addb      ,u                  Add in HDB-DOS offset hi-byte
               ldy       #(1*256)+0          Block count / reserved


* Get DSKCON opcode and branch accordingly

               lda       <DCOPC              Get DSKCON command
               IFDEF     SCSI
               beq       RESTOR2             Seek drive to LSN 0
               ENDC      
               suba      #2                  Read?
               beq       RBLOCK              Go if read opcode
               deca                          Write opcode?
               beq       WBLOCK              Yes, go write

               bra       FINIS               Invalid opcode, ignore


* Frequently Used Commands

               IFDEF     SCSI
REZERO         lda       #SEEK               Fast seek to LSN 0
               bsr       a@
               lda       #RSTR               Rezero opcode
a@             fcb       SKIP2               Do disk I/O
PARK           lda       #STSTOP             Start/ Stop (PARK)
               pshs      d,x,y,u             Save registers
               fcb       SKIP2

* The following are DSKCON internal commands

RESTOR2        lda       #SEEK               Seek to LSN 0
               clrb                          Reserved (0)
               ldx       #0                  Reserved
               tfr       x,y                 Clear Y (Reserved)
               fcb       SKIP2

               ENDC      

RBLOCK         lda       #RBLK               Read a block
               fcb       SKIP2
WBLOCK         lda       #WBLK               Write a block
               bsr       SETUP               Setup command packet
               fcb       SKIP2               Skip the following pshs
DISKIO         pshs      d,x,y,u             Save registers
               bsr       EXECMD              Execute command

FINIS          puls      u,y,x,d,pc          Done, restore & return


****************************************
* SCSI/IDE/DW START HERE
****************************************

               IFDEF     DW
SETUP          sta       <VCMD               Store command byte
               lda       IDNUM               Get drive number
               std       <VCMD+1             Store that and bits 23-16 of LSN
               stx       <VCMD+3             Store bits 15-0 of LSN
               rts       

* Exit: X = address of read/write buffer
SEND           ldy       #5
SENDY          ldx       #$FFD6              coco 2 normal speed address
               IFGT      Level-1
               lda       >$FFFC              MSB of NMI vector
               cmpa      #$FE                coco 3 uses FExx page
               bne       SETSPD              branch if not coco 3
               leax      3,x                 coco 3 fast speed address
               ENDC      
SETSPD         sta       ,x                  set speed
               ldd       <DCBPT              get read/write buffer
               ldx       #256                sector size
               pshs      x,d                 push receive params
               ldx       #VCMD               get pointer to command
               lbsr      DWWrite
               puls      x,y,pc              setup x,y and return

* Entry: A = DW opcode
EXECMD         pshs      cc                  save CC
               clr       <DCSTAT             assume no errors
               ldb       CCODE               get timeout code
               stb       RETRY               save off as retry count
               lda       <VCMD               Get command byte
               cmpa      #WBLK               WRITE command?
               bne       HREAD               branch if not

* Write away!
HWRITE         bsr       SEND
DoCSum         equ       *
               clra      
               clrb      
c2@            addb      ,x+
               adca      #0
               dec       <DCSTAT             use DCSTAT since it's 0, and we'll dec it to zero
               bne       c2@
               leax      -256,x
               pshs      d
               bsr       DWWrite
               ldy       #2
               leax      ,s
               bsr       DWWrite
               ldy       #$0001
               leax      ,s
               lbsr      DWRead
               puls      d
               bcs       BADEX
               bne       BADEX
               tsta                          ACK byte ok?
               beq       OKEX                branch if so
               cmpa      #E$CRC              checksum error?
               bne       BADEX
               lda       #OP_REWRIT
               sta       VCMD
               dec       RETRY               decrement retries
               bne       HWRITE              keep going if more
BADEX          lda       #$80
               sta       <DCSTAT
OKEX           puls      cc,pc


* Get 256 bytes of sector data
HREAD          bsr       SEND
               bsr       DWRead
               bcs       BADEX               branch if framing error
               bne       BADEX               branch if timeout
* Send two byte checksum
               pshs      y
               leax      ,s
               ldy       #1
               pshs      y,x
               leay      1,y
               bsr       DWWrite
* Get error code byte
               puls      x,y
               bsr       DWRead
               puls      d
               bcs       BADEX               branch if framing error
               bne       BADEX               branch if timeout
               tsta      
               beq       OKEX                if 0, we're ok
               cmpa      #243
               bne       BADEX
               dec       RETRY               decrement retries
               beq       BADEX
               lda       #RRBLK
               sta       VCMD
               bra       HREAD               and try getting sector again

               IFDEF     DW4
               use       dw4write.asm
               use       dw4read.asm
               ELSE      
               use       dwwrite.asm
               use       dwread.asm
               ENDC      

               setdp     $00

               ENDC      

               IFDEF     IDE
* SETUP - Setup the Command Packet
* Setup VCMD with command, LSN and option byte
* Entry: A = Command Code
*        B = Bits 23-16 of LSN
*        X = Bits 15-0  of LSN
SETUP          std       <VCMD               Store command byte and bits 23-16 of LSN
               stx       <VCMD+2             Store bits 15-0 of LSN
               lda       IDNUM
               lsla      
               lsla      
               lsla      
               lsla      
               ora       #DEVBYT
               sta       DRVSEL
               rts       


* IDE BUSY routine
* Waits for BUSY to be clear
* Exit: A = 0 (BUSY clear, OK); A = 128 (BUSY set, timeout)
* Note: calling routine can simply check of Z in CC is set/clear
W4NBUSY                  
               clra      
               ldb       CCODE
               ldy       #0
w@             tst       STATUS,u
               bpl       ok@
               leay      -1,y
               bne       w@
               decb      
               bne       w@
OOPS                     
               lda       #$80
ok@                      
               sta       <DCSTAT             and save
RET            rts       

* EXECMD - Execute all commands sent to controller
* Command packet at #VCMD ($F3)
* I/O buffer at DCBPT ($EE)
* Completion status in DCSTAT ($F0)
EXECMD         clr       <DCSTAT             Assume no error for now
               ldu       PORT                Point to data port
* Wait for !BUSY with timeout
               bsr       W4NBUSY
               bne       RET
ok@            ldx       #VCMD               Else point to command packet
               bsr       SEND                Send it out, returns command in A
               bcs       OOPS
DRQ@           ldb       STATUS,u            Get status from drive
               bitb      #ERROR              error?
               bne       OOPS                branch if error
               bitb      #DATARQ             DATARQ bit set?
               beq       DRQ@                continue checking if not
               ldx       <DCBPT              get read/write buffer
               cmpa      #WBLK               WRITE command?
* BETTER BE READ OR RCAPY COMMAND HERE!!!
               bne       HREAD               Branch if not

* We write all 512 bytes starting at X...
HWRITE         clr       ,-s
WRITLP         ldd       ,x++
               stb       LATCH,u
               sta       DATARW,u
               dec       ,s
               bne       WRITLP
               puls      a,pc

* IMPORTANT! THIS CODE CANNOT CHANGE!!!!
*
* This routine is constructed to allow a full 512 byte sector
* read *ONLY* if the read buffer starts at $600.  Since WRITE sends
* all 512 bytes of a sector to the drive, we read a full 512 bytes
* IF the read starts at $600 (DBUF0).  This is an attempt to preserve
* the entire 512 bytes of a drive's sector in that case.
* This is particularly true of DSKI$/DSKO$, which performs
* reads/writes at $600, and is used when running LINK.BAS.
*
* If the read buffer is not at $600, we only read the first
* 256 bytes, and throw away the remaining.
HREAD          bsr       r1@
               cmpx      #$700               are we now at DBUF1 address?
               beq       r1@                 branch if so
* Waste 256 bytes here
* Here, A = 128...
r0@            ldb       DATARW,u
               deca      
               bne       r0@
               rts       
r1@            clr       ,-s                 Clear counter on stack
rl@            lda       DATARW,u
               ldb       LATCH,u
               std       ,x++
               inc       ,s
               bpl       rl@
               puls      a,pc

* SEND - CALCULATE SECTOR FROM CHS AND SEND TO IDE DRIVE
*        EXIT: CARRY = 0 OK, CARRY = 1 ERROR
*              A = COMMAND WRITTEN TO IDE
SEND                     
* As per IDE spec, we must select the drive (master/slave)
* that we wish to talk to.
               ldy       #0                  set up timeout
               lda       DRVSEL
               sta       DEVHED,u
a@             leay      -1,y
               bne       b@
               comb                          set carry
               rts                           return with error
b@             lda       STATUS,u            get status byte from drive
               bmi       a@                  branch if BUSY is set
               lsla                          else put DRDY bit 6 into bit 7
               bpl       a@                  and try again if clear

               IFDEF     USELBA
* LBA mode is easy... just put the LSN out in the registers
               ldb       <VCMD+1
               stb       CYLHI,u
               ldd       <VCMD+2
               stb       SECTNM,u
               sta       CYLLO,u
               ldb       #$01
               stb       SECTCN,u
               ENDC      

               IFDEF     USECHS
* CHS is hard... we have to compute the CHS value based on the LSN 
               ldd       NHEADS              get heads and sectors
               mul                           multiply
               pshs      d                   save result
               ldd       2,x                 get LSN bits 15-0
               ldy       #-1                 start out -1
               inc       1,x
A@             leay      1,y
               subd      ,s
               bhs       A@
               dec       1,x
               bne       A@
               addd      ,s++
               pshs      d
               tfr       y,d
               exg       a,b
               std       CYLLO,u
               puls      d
               ldy       #-1
B@             leay      1,y
               subb      NSECTS
               sbca      #0
               bcc       B@
               addb      NSECTS
               incb      
               stb       SECTNM,u
               ldb       #$01                1 block
               stb       SECTCN,u
               tfr       y,d
               orb       DRVSEL
               stb       DEVHED,u
               ENDC      

               lda       ,x
               sta       CMD,u               Lastly, push the command to the drive
               clrb                          clear carry
               rts       

               ENDC      

******** END OF IDE



******** START OF SCSI

               IFDEF     SCSI
* SETUP - Setup the Command Packet
*
* Sets up VCMD with command, LSN and option byte
* ENTRY: B,X = Bits 23-0 of LSN
SETUP          pshs      u,d                 B may be modified
               ldu       #VCMD               Get pointer to command buffer
               sta       ,u+                 Store command byte first

               lda       DRVSEL              Get LUN
               lsla                          Shift over to appropriate place
               lsla      
               lsla      
               lsla      
               lsla      
               pshs      a                   Save LUN bits
               andb      #%00011111          Clear upper 3 bits (LUN)
               orb       ,s+                 OR in LUN bits

               stb       ,u+                 Store sector hi-byte
               stx       ,u++                Store sector lo-word
               sty       ,u                  Store options

               puls      u,d,pc              Restore & return

* WAKEY (SCSI) - Get controller on the bus
*   Wait for BUS FREE and put SCSI id on bus
* EXIT: Z = 1 NOT OK, Z = 0 OK
WAKEY          lda       #$80                Not ready code
               sta       <DCSTAT             Store error code; we assume an error
               ldx       #0                  Bus free timer lo
               ldu       PORT                Point to data port
WAKE           lda       STATUS,u            Read SCSI status
               bita      #BSY+SEL            Bus Free detected?
               beq       WAKEUP              Yes, attempt selection
               leax      -1,x
               bne       WAKE
               rts       
WAKEUP         clra      
               ldb       IDNUM
               andb      #$07
               orcc      #$01                set carry
a@             rola      
               decb      
               bpl       a@
               ora       #$80                for SCSI-3 compliant drives
               sta       ,u                  put to H/W
               bsr       c@                  delay for a bit
               sta       SELECT,u            select it
b@             ldx       #0
bb@            lda       STATUS,u            get status
               bita      #BSY                BSY set?
               bne       c@                  branch if so
               leax      -1,x
               bne       bb@
c@             rts                           Yes, drive on-line

* EXECMD - Execute all commands sent to controller
* Command packet at #VCMD ($F3)
* I/O buffer at DCBPT ($EE)
* Completion status in DCSTAT ($F0)
EXECMD         lda       CCODE               Timeout code
a@             pshs      A
COMAND         bsr       WAKEY               Get controller on the bus
               beq       c@                  Branch if unsuccessful
               ldx       #VCMD               Point to command packet
               bsr       SEND                Send command
               bsr       WAITRQ              Wait -REQ & get status
CMDIN          bita      #CMD                Command or data?
               bne       GETSTA              Not data phase, get status
GRABDT         ldx       <DCBPT              Get DSKCON buffer pointer
               bita      #INOUT              "DATA IN" or "DATA OUT"?
               beq       b@                  Not asserted = "DATA OUT"
r@             bsr       HREAD               Asserted = "DATA IN"
               fcb       SKIP2               Skip write
b@             bsr       HWRITE              Send data out to target
GETSTA         bsr       INSTAT              Get completion status
               bita      #BUSY               Was target busy?
               bne       COMAND              Yes, resend command
               bita      #ERROR              Any errors?
c@             puls      A
               beq       DONE                No, return
* Arrive here if there was an error
               deca      
               bne       a@
               bsr       WAKEY               Get controller on the bus
               beq       DONE
               lda       #RDET               Request sense command
               clrb                          Reserved (0)
               ldx       #0                  Reserved (0)
               tfr       x,y                 Clear Y (Reserved 0)
               lbsr      SETUP               Setup the command packet
               ldx       #VCMD               Point to command packet
               bsr       SEND                Send command
               ldx       #VCMD               Point to command/error RAM
               bsr       HREAD               Read error info into RAM
               bsr       INSTAT              Get completion status
               lda       <VCMD               Get error code
               anda      #$7F
               cmpa      #$70
               bne       d@
               lda       <VAD1
d@             sta       <DCSTAT             Store for DSKCON
DONE           rts                           Return

SEND           bsr       WAITRQ              Wait for -REQ asserted
               bita      #CMD                Comand or data phase?
               beq       DONE                Data phase, exit
               bita      #INOUT              Test data direction
               bne       DONE                Status phase, exit
               lda       ,x+                 Get data to send
               sta       ,u                  Send it
               bra       SEND                Go back for more


* Wait for controller to assert -REQ

WAITRQ         lda       STATUS,u            Read SCSI status
               IFDEF     ACK
               bita      #ACK                -ACK asserted?
               bne       WAITRQ
               ENDC      
               bita      #REQ                -REQ asserted?
               beq       WAITRQ              No, check again
               rts                           Yes, return

HREAD          clrb      
READ2          bsr       WAITRQ              Wat for -REQ asserted
               bita      #CMD                Command or data?
               bne       DONE                Command phase, exit
               lda       ,u                  Get a data byte
               sta       ,x+                 Store in buffer
               decb      
               bne       READ2
* This next check allows a full 512 byte sector read ONLY
* if the read buffer is at $600.  This gets around a nasty
* problem with 512 byte sector drives and OS-9.
               cmpx      #$700               DBUF1 address?
               beq       READ2
READ3          bsr       WAITRQ
               bita      #CMD
               bne       DONE
               lda       ,u
               bra       READ3

HWRITE         bsr       WAITRQ              Wait for -REQ asserted
               bita      #CMD                Command or data?
               bne       DONE                Command phase, exit
               lda       ,x+                 Get a byte from buffer
               sta       ,u                  Send it out
               bra       HWRITE              Go back for more

* Get status and message bytes

INSTAT         bsr       WAITRQ              Wait for -REQ asserted
               lda       ,u                  Get status byte
               anda      #$0F                Mask unused bytes
               sta       <DCSTAT             Store status for DSKCON
               bsr       WAITRQ              Wait for -REQ asserted
               clra                          A=0
               sta       ,u                  Clear output latch & ACK
               lda       <DCSTAT             Restore status
               rts                           Return with status

               ENDC      

******** END OF SCSI



* Generate a "BEEP" sound


* Old "BEEP" routine has been superceded by use of Color BASIC sound routine to shrink space (thanks Darren A.)
BEEP           pshs      u,x,d               PRESERVE REGISTERS
               ldd       #208*256+1          SOUND PITCH AND DURATION
               sta       $008C               STORE SOUND PARAMS
               jsr       LA951               CALL COLOR BASIC SOUND ROUTINE
               puls      d,x,u,pc            RESTORE REGISTERS AND RETURN

*BEEP           pshs      u,x,d,cc            Save registers
*               orcc      #$50                Interrupts off
*               clrb                          SOURCE	Zero is 6 bit DAC
*               jsr       LA9A2               Select sound source in B
*               jsr       LA976               Set "AUDIO ON"
*               ldb       #$A0                Beep duration
**         ldx   #$FF20     Read PIA (DAC Port)
*
*FLIP                     
**         lda   ,x         Read PIA (DAC Port)
*               lda       $FF20               Read PIA (DAC Port)
*               eora      #$80                Flip hi-bit
**         sta   ,x         Store data
*               sta       $FF20               Store data
*               bsr       DELAY               Go do a delay (tone pitch)
*               decb                          Decrement duration count
*               bne       FLIP                Not done, do more
*
*               jsr       LA974               Do "AUDIO OFF"
*               puls      cc,d,u,x,pc         Restore registers & return

DELAY          pshs      b                   Save B (tone duration)
               ldb       #$60                Get pitch delay
a@             decb                          Count it down
               bne       a@                  Not done, count more
               puls      b,pc                Restore duration & return

* Set directory size by screen width

SETSIZ         lda       <$E7                Get screen mode
               suba      #2                  WIDTH 80?
               beq       COL5                Yes, do 5 columns

* Otherwise, do 2 columns

               lda       #2                  Two columns
               fcb       SKIP2               Skip lda #5
COL5           lda       #5                  Five columns
               sta       DIR1                Store in temp RAM

* Break DIR into columns

NEWCOL         lda       DIR1                Get column value
               sta       DIR2                Reset column counter
               jmp       LB958               Send a C/R & return

DCOUNT         dec       DIR2                Decrement column counter
               beq       NEWCOL              All done, send a C/R
               jmp       LB9AC               Else send a space

* Point X TO Drive FAT, ?OB ERROR if none left

GETFAT         pshs      D                   Save registers
               lda       <DCDRV              Get drive number
               ldb       #4                  4 tables
               ldx       #FATS               Point to FATs
a@             cmpa      2,x                 Is this table ours?
               beq       d@                  Yes, keep this table
               leax      <$4A,x              Bump up to next FAT
               decb                          Decrement table counter
               bne       a@                  Check next table
* Allocate the first free table in drive to A
               ldb       #4                  4 tables
               ldx       #FATS               Point to FATs
b@             tst       ,x                  Any files active?
               beq       c@                  No, allocate table
               leax      <$4A,x              Yes, bump to next FAT...
               decb                          ...decrement counter
               bne       b@                  Try next table
               jmp       LC504               None left, ?OB ERROR
c@             sta       2,x                 Store drive number
d@             puls      d,pc                Pull registers & return


* Parse & calculate a multi-char drive number

NUMCAL         cmpb      #$01
               beq       b@
               clr       <DCDRV              now DCDRV will always have some drive number
a@             decb      
               beq       b@
               lda       ,u+                 Get next character
               suba      #'0                 Subtract ASCII
               bcs       b@                  Lower than 0, skip it
               cmpa      #9                  Within range?
               bhi       b@                  No, skip it
               pshs      d                   Save string len & number
               lda       <DCDRV              Get number
               ldb       #10                 Times ten
               mul                           Multiply
               addb      ,s
               stb       <DCDRV              Store tens number
               adca      #0
               puls      d                   Restore string length & number
               beq       a@                  ?FC ERROR if > 255
FCERR          jmp       LB44A
b@             rts                           Return


* Check for enhanced drive commands

DRVCHK         cmpa      #TOKEN_ON           "ON" token?
               beq       DRIVEH              Yes, enable hard drives 0-3
               cmpa      #TOKEN_OFF          "OFF" Token?
               beq       DRIVEF              Yes, enable floppys
               cmpa      #TOKEN_STOP         "STOP" token?
               beq       DPARK               Yes, go park drive
               cmpa      #TOKEN_RESTORE      "RESTORE" token?
               beq       RECAL               Yes, rezero drive
               cmpa      #'#                 "DRIVE #" command?
               beq       DNUM                Yes, set SCSI or IDE ID

* If here, must be a drive number
               jsr       EVALEXPB            Evaluate argument
               jsr       LA5C7               ?SN ERROR if more chars
               cmpb      MAXDRV              Valid number requested?
               jmp       LCECA               Store default drive & return


* Enable 0-3 hard drive or floppy
DRIVEH         clrb                          No offset = hard drive
               fcb       SKIP2               Skip 2 bytes
DRIVEF         ldb       #4                  Offset of 4 = floppies
               jsr       <$9F                Parse over on/off token
               beq       a@                  Go if no argument
               tstb                          "DRIVE ON" command?
               beq       a@                  Yes, argument not allowed
               jsr       EVALEXPB            No, evaluate argument
               incb                          Correct it
               cmpb      #4                  Legal number?
               bhi       FCERR               No, ?FC ERROR
a@             jsr       LA5C7               ?SN ERROR if more chars
DCLOSE         bsr       HCLOSE              Close all disk files
               stb       HDFLAG              Store hard drive flag
               rts                           Return

* Park drive in landing zone
DPARK                    
               IFDEF     SCSI
               bsr       RECAL               Rezero drive
               jsr       >PARK               Park it
               bra       b@                  Check for error & return
               ENDC      
** Recalibrate (rezero) drive
RECAL          jsr       <$9F                Drive number specified?
               beq       a@                  No, just park
               bsr       DSET05              Yes, get number & setup
a@             jsr       LA5C7               ?SN ERROR if more chars
               bsr       HCLOSE              Close all disk files
               IFDEF     SCSI
               jsr       >REZERO             Reset drive to (00)
b@             tst       <DCSTAT             Any errors?
               bne       IOERR               Yes, ?IO ERROR
               ENDC      
               rts                           Return

* Close all files
HCLOSE         pshs      d,u,x,y             Save registers
               jsr       DVEC7               Close all files
               puls      d,u,x,y,pc          Restore & return

IOERR          jmp       LD709

* Select Device ID number
DNUM           jsr       <$9F                Parse over "#"
DSET05         jsr       EVALEXPB            Evaluate argument
               cmpb      #MAXDN-1            Legal?
               bhi       FCERR               No, ?FC ERROR
               jsr       LA5C7               ?SN ERROR if more chars
               jsr       HCLOSE              Close all disk files
               lda       IDNUM               Get current ID
               pshs      a                   And save it on stack
               stb       IDNUM               Store selected drive ID
               clr       $95A                Reset default drive to 0
               bsr       GETMAX
               puls      b
               tst       <DCSTAT             Errors?
               beq       GOBACK
               stb       IDNUM               Restore old drive
               bra       IOERR
GOBACK         rts       

               IFDEF     DW
GETMAX         clr       <DCSTAT
               ldx       #VCMD
               ldd       #$FFFF              for DriveWire, assume maximum size ($FFFFFF)
               std       1,x
               sta       3,x
               ENDC      

               IFDEF     SCSI
* GETMAX (SCSI)
GETMAX         lda       #RCAPY              Read Capacity command
               clrb                          RESERVED
               ldy       #0
               sty       <VEXT
               sty       <VEXT+2
               lbsr      SETUP               Setup command
               ldx       #VCMD               Point to buffer in RAM
               stx       <DCBPT              Tell DSKCON
               leax      2,x                 Point past command
               ldd       #(8*256)+0          Write 8 bytes of 0
               jsr       >LD6C2               Write rest of command
               lbsr      EXECMD              Execute command
               tst       <DCSTAT             Any errors?
               bne       GOBACK
               ldx       #VCMD
               ENDC      

               IFDEF     IDE
* GETMAX (IDE)
GETMAX                   
               ldd       #$0600              Get buffer #0 address
               std       <DCBPT              Save in DSKCON buffer pointer
               lda       #RCAPY
               ldx       #$0000
               lbsr      SETUP
               lbsr      EXECMD
               tst       <DCSTAT
               bne       GOBACK
               ldy       <DCBPT
               IFDEF     USELBA
               ldx       #NCYLS
               lda       99,y                Get mode byte
               bita      #%00000010          LBA mode?
               lbeq      OOPS                branch if not... error
               sta       ,x                  Save off mode byte
               ldd       121,y
               sta       2,x
               stb       1,x
               lda       120,y
               sta       3,x
               ENDC      
               IFDEF     USECHS
               ldx       #VCMD
               ldd       2,y                 Get cylinder count
               exg       a,b
               std       NCYLS
               lda       6,y                 Get head count
               sta       NHEADS
               lda       12,y                Get byte count
               sta       NSECTS
* COMPUTE C*H*S
               clr       ,x
               clr       1,x
               ldd       NHEADS              Get heads/sectors
               mul       
               std       4,x                 Save off
               lda       NCYLS+1
               mul       
               std       2,x
               lda       NCYLS+1
               ldb       4,x
               mul       
               addd      1,x
               std       1,x
               bcc       XMU10
               inc       ,x
XMU10          lda       NCYLS
               ldb       5,x
               mul       
               addd      1,x
               std       1,x
               bcc       XMU20
               inc       ,x
XMU20          lda       NCYLS
               ldb       4,x
               mul       
               addd      ,x
               std       ,x
               ENDC      
               ENDC      

* Subtract HDB-DOS start sector from available sectors
CALCOS         ldu       #HDBHI
               ldd       2,x                 Get size lo-word
               addd      #$01
               bcc       A@
               inc       1,x
A@             subd      1,u
               bcc       B@                  Go if no underflow
               dec       1,x                 Decrement hi-byte
B@             std       2,x                 Save new lo-word
               lda       1,x                 Get hi-byte
               suba      ,u                  Subtract hi-byte
               sta       1,x                 Store new hi-byte

* This routine calculates MAXDRV from available sectors

               clr       ,-s                 Setup a counter
C@             ldd       2,x                 Get sector lo-word
               subd      #(18*35)            Subtract out 1 drive
               bcc       A@                  Go if no underflow
               tst       1,x                 Test MS byte
               beq       B@                  Exit if it was zero
               dec       1,x                 Decrement MS byte
A@             std       2,x                 Store new lo-word
               inc       ,s                  Increment max drive count
               bne       C@                  Go again if no overflow
B@             dec       ,s                  Normalize to base 0
               puls      B                   Get max drive
               stb       MAXDRV              Store it
               clrb      
DUN            rts                           Return


* The DIR command enters here

DIRCHK         jsr       >LD24F               Get first drive number
               jsr       LA5C7               Any more chars?
DNAME          bsr       SETVAR              Select track & sector 17
               lda       #2                  Read opcode
               sta       <DCOPC              Tell DSKCON
               ldx       #DBUF0              Point to buffer
               stx       <DCBPT              Tell DSKCON
               jsr       >LD6F2               Read diskname
               lda       ,x                  Get first character
               beq       DUN                 NULL? Exit
               coma                          Invert
               beq       DUN                 $FF? Exit
               bsr       SENDCR              Send a C/R
a@             lda       ,x+                 Get a character
               beq       DUN                 Exit if done
               jsr       >LCCFD               R.G. See above jmp [$A002] 
               bra       a@                  And get another


* More of directory code.
BOISY          beq       a@
               lbcs      LCCBB
a@             lda       $01D4
               cmpa      $01D5
               beq       b@
               bsr       SENDCR
b@             ldx       #DRVMSG-1           DRIVE=
               jsr       STRINOUT
               clra      
               ldb       <$EB
               jsr       LBDCC
               ldx       #FREMSG-1           FREE=
               jsr       STRINOUT
               bsr       BRIAN
               clra      
               jsr       LBDCC
SENDCR         jmp       LB958
BRIAN          jsr       >LC755
               leax      6,x
               clr       ,-s
               ldb       #$44
HERE@          lda       ,x+
               coma      
               bne       D@
               inc       ,s
D@             decb      
               bne       HERE@
               puls      pc,b
SETVAR         ldd       #$1111
               std       <$EC
               rts       

* Rename drive command syntax: RENAME DRIVE n, "STRING"

RENAME2        cmpa      #TOKEN_DRIVE        "DRIVE" token?
               bne       NONAME              No, continue old code
               jsr       <$9F                Yes, parse over it
               jsr       >LD256               Get drive number
               jsr       SYNCOMMA            Syntax check for comma
               ldd       #(0*256)+$FF        Write 256 bytes of $FF
               ldx       #DBUF0              Point to buffer
               stx       <DCBPT              Tell DSKCON where buffer is
               jsr       >LD6C2               Go fill buffer
               jsr       L8748               Evaluate string (DISKNAME)
               jsr       LA5C7               ?SN ERROR if more chars
               tstb                          See if NULL string ("")
               beq       NULL                Null, copy nothing
               ldu       <DCBPT              Point U to disk buffer
               jsr       LA59A               Copy string to disk buffer
               clr       ,u                  NULL terminator
NULL           bsr       SETVAR              Select track & sector
               lda       #3                  Write opcode
               sta       <DCOPC              DSKCON opcode write
DOIO           jmp       LD6F2               Write buffer to disk
NONAME         ldx       <$A6                Continue...
               pshs      X                   ...old code
               jmp       LD01F               Jump back into old code



* Patch to allow RUNM command

RUNM           cmpa      #'M                 "RUNM"?
               lbne      DVEC18              No, continue old code
               pshs      CC
               orcc      #$50                Yes, shut off interrupts
               ldx       #LA545              ...EXEC
               stx       1,s
               jsr       >LCFC1               Do a LOADM...
               clr       $FF40               Shut off drive motor(s)...
               puls      cc,pc               ...EXEC


* Test for COPY to string or drive number

COPtst         ldx       <$A6                Get basic input pointer
               pshs      X                   Save it
               jsr       LB156               Evaluate expression
               ldx       #$1A9               Start of string stack
               stx       <$0B                Reset string stack
               puls      X                   Restore input pointer
               stx       <$A6                Restore pointer
               tst       <$6                 Expression numeric?
               lbeq      LD256               Yes, evaluate drive number
               ldx       #$954               No, point at ext. buffer
               jmp       LC938               And get filename string


* Allow "COPY" command to overwrite existing file

AETEST         jsr       >LC68C               Scan dir for filename
               tst       $973                Already exist?
               beq       AE10                No, just return
ASK            lda       <$1A,s              R.G. stack holds data for COPY $1A=26 is part of Source Name
               cmpa      $0D,s
               bne       a@
               jmp       LD05C
a@             lda       <$68                Get BASIC line # MSB
               inca                          Direct mode ($FF)?
               bne       d@                  No, just overwrite
               ldx       #OVRMSG-1           Yes, point to message
               bsr       b@                  Print it
               bsr       GETY                Check for (Y)ES
               beq       c@                  Yes, overwrite file
               leas      <$26,s              No, remove temp variables
               ldx       #ABTMSG-1           "ABORTED" message
b@             jmp       STRINOUT            Print it & return
c@             jsr       >LCCFD               R.G. See above print Y
d@             jmp       LC6F5               Kill dest file & return


* Get a keypress, beq if it's a "Y"

GETY           jsr       GETKEY              Get a key
               anda      #$FF-$20            Force upper case
               cmpa      #'Y                 Set flag for "Y"
AE10           rts                           Return


* Prompt Messages

FORMT          fcb       CR
               fcc       "ERASE DISK "
               fcb       STOP2

RDYMSG         fcb       CR
               fcc       "ARE YOU SURE? (Y/N) "
               fcb       STOP2

ABTMSG         fcc       "ABORTED"
               fcb       STOP2

OVRMSG         fcb       CR
               fcc       "FILE ALREADY EXISTS"
               fcb       CR
               fcc       "OVERWRITE? (Y/N) "
               fcb       STOP2

FREMSG         fcc       "  FREE="
               fcb       STOP2

DRVMSG         fcc       "DRIVE="
               fcb       STOP2

               IFNE      DW-1
FAILED         equ       *
               IFDEF     USELBA
               fcc       "LBA "
               ENDC      
               fcc       "HARD DRIVE NOT FOUND"
               fcb       CR
               fcb       CR
               fcb       STOP2
               ENDC      


* AUTOEXEC Filename

NAMBUF         fcc       "AUTOEXEC"          FILENAME
               fcc       "BAS"               EXTENSION


* Main Sign-On Message And Copyright Notice

SIGNON         fcc       "HDB-DOS "
               fcb       VMAJOR+$30,$2E,VMINOR+$30
               IFNE      VREV
               fcb       VREV
               ENDC      
               fcb       $20
               IFDEF     IDE
               IFDEF     USELBA
               fcc       "LBA"
               ENDC      
               IFDEF     USECHS
               fcc       "IDE"
               ENDC      
               ENDC      
               IFDEF     TC3
               fcc       "TC^3"
               ENDC      
               IFDEF     KENTON
               fcc       "KENTON"
               ENDC      
               IFDEF     LRTECH
               fcc       "LRTECH"
               ENDC      
               IFDEF     DHDII
               fcc       "HD-II"
               ENDC      
               IFDEF     D4N1
               fcc       "4-N-1"
               ENDC      
               IFDEF     DW
               IFDEF     ARDUINO
               fcc       "DW3 ARDUINO"
               ELSE
               IFDEF     DW4
               fcc       "DW4 COCO "
               ELSE
               IFDEF     BECKER
               fcc       "BECKER COCO "
               ELSE
               IFDEF     BECKERTO
               fcc       "BECKER COCO "
               ELSE
               IFDEF     JMCPBCK
               fcc       "J&M CP COCO "
               ELSE
               fcc       "DW3 COCO "
               ENDC
               ENDC
               ENDC
               ENDC
               IFGT      Level-1
               fcc       "3"
               ELSE      
               IFDEF     BAUD38400
               fcc       "1"
               ELSE      
               fcc       "2"
               ENDC      
               ENDC      
               ENDC      
               ENDC      
               IFDEF     DRAGON
               fcc       " ON DRAGON"
               ENDC
*	fcc	" (C) 2002 AE"
               fcb       CR,CR
               fcb       STOP2

* Canned command for read, drive 0, track 0, sector 1

DTEST          fdb       $0200               Read, drive 0
               fdb       $0001               Track 0, sector 1
               fdb       DBUF0               DSKCON buffer to use


ATARI          tst       <$6F
               beq       A@
               blt       C@
               jmp       LCC22
A@             cmpa      #$07
               bne       B@
               jsr       >BEEP
B@             cmpa      #$0C
               bne       C@
               pshs      u,x,y,b,a,cc
               clra      
               jsr       CLS
               puls      u,x,y,b,a,cc
C@             jmp       XVEC3               Console out

* FlexiKey Copyright (C) 1984 by Colin J. Stearman
* Source code reproduced from Rainbow - October 1984

FLEXKY         lda       <$6F                Get device number
               beq       KEY                 Go if keyboard
               cmpa      #-1                 Doing cassete I/O?
               bne       JMPOUT              No, go to original code
               nega                          Yes, flag buffer in use
               sta       INTFLG              Store flag
JMPOUT         jmp       DVEC4               Jump to old code
KEY            pshs      X,B                 Save registers
               ldx       7,s                 Where are we coming from?
               cmpx      #LA39A+3            The idle loop?
               beq       INIDLE              Yes, do FlexiKey
               puls      b,x                 No, restore registers...
               bra       JMPOUT              ...and go to old code
INIDLE         clr       <$70                Flag buffer flushed
               tst       INTFLG              Been here since last C/R?
               beq       GETTKN              No, clear flags
               bmi       TESTWH              Not cassette I/O so continue
               clr       HLDBFR              Mark first byte as 0
               clr       INTFLG              Ready for complementing
GETTKN         com       INTFLG              Set to $FF
RENTER         clr       HLDPTR              Clear flags
               clr       INSERT              Clear flags
               clr       WHLINE              Clear flags
KYREAD         jsr       >GETKEY             Wait for a key & blink cursor
               cmpa      #9                  Right arrow?
               beq       GETCHR              Yes, get a char from buffer
               cmpa      #$5D                SHIFT-Right arrow?
               bne       a@                  No, see what it is
               com       WHLINE              Yes, set "WHOLE LINE" flag
               bra       GETCHR              Get next char from buffer
a@             cmpa      #$5F                SHIFT-Up arrow?
               bne       b@                  No, see what it is
               com       INSERT              Yes, set insert flag
TESTWH         tst       WHLINE              Send out entire buffer?
               beq       KYREAD              No, go get them from keyboard
GETCHR         clr       INSERT              Reset insert flag
               ldb       HLDPTR              Get character pointer
               ldx       #HLDBFR             Point X to hold buffer
               abx                           ADD	Offset to X
               lda       ,x                  Get character
               bne       GOODCH              Not end flag so continue
               clr       WHLINE              At end so reset entire line flag
               bra       KYREAD              And get more from keyboard
GOODCH         inc       HLDPTR              Point to next char
               bra       EXIT                And return with char
b@             cmpa      #$5B                SHIFT-Down? (Was SHIFT-@)
               beq       LINCLS              Yes, goto line close
               cmpa      #CR                 ENTER Key?
               beq       ENTER               Yes, goto ENTER
               cmpa      #8                  Backspace?
               beq       c@                  Yes, delete last char
               cmpa      #$0A                Down arrow?
               bne       d@                  No, see what it is
               inc       HLDPTR              Bump up hold pointer
               bra       KYREAD              Go read the keyboard
c@             tst       INSERT              Is insert on?
               bne       CONXIT              Yes, so don't increment
               bsr       DECPNT              No, decrement hold pointer
               bra       CONXIT              And exit
DECPNT         tst       HLDPTR              Is pointer at zero?
               beq       ATZERO              Yes, don't try to decrement
               dec       HLDPTR              No, decrement it
ATZERO         rts                           Return
d@             cmpa      #$15                SHIFT-Backspace?
               beq       CLRPNT              Yes, reset hold pointer
               cmpa      #$0C                Clear key?
               beq       CLRPNT              Yes, reset hold pointer
               cmpa      #3                  BREAK key?
               beq       CLRPNT              Yes, reset hold pointer
               cmpa      #$5C                SHIFT-Clear?
               bne       CONXIT              No, not special insert
               jsr       >GETKEY             Special insert, get key
               bra       CONXIT              And exit with it
CLRPNT         clr       HLDPTR              Clear hold pointer
CONXIT         cmpa      #32                 Is it a control character?
               blo       EXIT                Yes, so just exit
               tst       INSERT              No, test insert flag
               bne       EXIT                Insert off so exit
               inc       HLDPTR              Bump up hold pointer
EXIT           puls      b,x                 Restore registers
               leas      2,s                 Purge return
               rts                           Return
LINCLS         clr       [1,s]               Zero out last byte
               lda       #95                 ASCII "_" (was "@")
               jsr       >LCCFD               Print it
               jsr       LB958               Send a C/R (NEXT LINE)
               ldb       #1                  Reset BASIC's character count
               stb       ,s                  Store new count
               ldx       #BASBFR             Point to BASIC line input buffer
               stx       1,s                 Store address for BASIC
               bsr       MOVBLK              Transfer BASIC buffer to hold buffer
               lbra      RENTER              Go do it all over again!
ENTER          clr       INTFLG              Flag BASIC buffer changed
               clr       [1,s]               Set an end (0) flag
               bsr       MOVBLK              Transfer BASIC buffer to hold buffer
               bra       EXIT                And exit
MOVBLK         ldx       #BASBFR             Point to BASIC buffer
               ldy       #HLDBFR             Point to hold buffer
DOMORE         ldb       ,x+                 Get a byte
               stb       ,y+                 Put a byte
               bne       DOMORE              If not end (0) flag get more
               rts                           ALL	Done, return
ZZLAST         equ       *-1                 Cannot be > $DFFF!



               fill      $39,MAGICDG+$2000-*

               end LC00C
