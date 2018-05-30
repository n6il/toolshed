*****************************************
*  SDC Driver
*  Most of the important code was taken
*  from Darren Atkinson's
*  CoCoSDC Technical Info Guide
*****************************************

*********************************************************************
***    Hardware Addressing
*********************************************************************
CTRLATCH    equ    $FF40          ; controller latch (write)
CMDREG      equ    $FF48          ; command register (write)
STATREG     equ    $FF48          ; status register (read)
PREG1       equ    $FF49          ; param register 1
PREG2       equ    $FF4A          ; param register 2
PREG3       equ    $FF4B          ; param register 3
DATREGA     equ    PREG2          ; first data register
DATREGB     equ    PREG3          ; second data register


;;;  wait for controller to be ready ( not busy )
wait        pshs   x,a            ; preserve registers
            ldx    #0             ; long timeout counter = 65536
loop@       lda    STATREG        ; read status
            lsra                  ; move BUSY bit to Carry
            bcc    ret@           ; branch if not busy
            leax   -1,x           ; decrement timeout counter
            bne    loop@          ; loop if not timeout
ret@        puls   a,x,pc         ; restore registers and return

;;; oops command failed
oops        lda    #0x80
            sta    <DCSTAT
            rts


;;;
;;;
;;;  Below is the external HDB Interface Methods
;;;
;;;

* SETUP - Setup the Command Packet
* Setup VCMD with command, LSN and option byte
* Entry: A = Command Code
*        B = Bits 23-16 of LSN
*        X = Bits 15-0  of LSN
SETUP       ldy    #PREG2         ; set Y to point at Data Register A
            sta    <VCMD          ; store opcode
            stb    -1,y           ; store LSN in the three..
            stx    ,y             ; ..Block Address registers
            ldb    #$43           ; write $43 to the controller..
            stb    CTRLATCH       ; ..latch to select Enhanced Mode
            rts

* EXECMD - Execute all commands sent to controller
* Command packet at #VCMD ($F3)
* I/O buffer at DCBPT ($EE)
* Completion status in DCSTAT ($F0)
* Entry: A = DW opcode
EXECMD      clr    <DCSTAT        ; assume no errors
            lda    <VCMD          ; Get command byte
            cmpa   #WBLK          ; WRITE command?
            bne    HREAD          ; branch if not
HWRITE
            ldy    #PREG2         ; set Y to point at Data Register A
            lda    IDNUM          ; Get No.
            ora    #$A0           ; setup Write Sector command for target device
            sta    -2,y           ; send to command register (FF48)
            exg    a,a            ; some time to digest the command
*** Wait for Controller Ready or Failed.
            ldx    #0             ; long timeout counter = 65536
wrWait      lda    -2,y           ; read controller status
            bmi    wrFail         ; branch if FAILED bit is set
            bita   #2             ; test the READY bit and..
            bne    wrRdy          ; ..branch if ready
            leax   -1,x           ; decrement timeout counter
            bne    wrWait         ; continue polling until timeout
wrFail      clr    CTRLATCH       ; return controller to emulation mode
            ldb    #$80
            stb    <DCSTAT        ; mark return reg for error
            rts                   ; restore registers and return
*** Controller Ready. Write Sector Data. Use partial loop unrolling.
wrRdy       ldx    DCBPT          ; move data pointer from U to X
            ldd    #64*256+4      ; A = chunk count (64), B = bytes per chunk (4)
wrChnk      ldu    ,x             ; get 2 data bytes from source
            stu    ,y             ; send data to controller
            ldu    2,x            ; two more bytes..
            stu    ,y             ; ..for this chunk
            abx                   ; increment X by chunk size (4)
            deca                  ; decrement loop counter
            bne    wrChnk         ; loop until all chunks written
*** Wait for operation to complete and check status
wrComp      lda    -2,y           ; read controller status
            bmi    wrFail         ; branch if FAILED bit is set
            lsra                  ; move BUSY bit into carry
            bcs    wrComp         ; loop if still busy
*** Return Success.
            clr    CTRLATCH       ; clear carry / return controller to emulation mode
            rts                   ; restore registers and return

HREAD
            ldy    #PREG2         ; set Y to point at Data Register A
            lda    IDNUM          ; A = real HDB drive number
            ora    #$80           ; setup Read Sector command for target device
            sta    -2,y           ; send to command register (FF48)
            exg    a,a            ; some time to digest the command
*** Wait for Controller Ready or Failed.
            ldx    #0             ; long timeout counter = 65536
rdWait      lda    -2,y           ; read controller status
            bmi    rdFail         ; branch if FAILED bit is set
            bita   #2             ; test the READY bit and..
            bne    rdRdy          ; ..branch if ready
            leax   -1,x           ; decrement timeout counter
            bne    rdWait         ; continue polling until timeout
rdFail      clr    CTRLATCH       ; return controller to emulation mode
            ldb    #$80
            stb    <DCSTAT        ; set carry for failure
            rts                   ; restore registers and return

*** Controller Ready. Read the Sector Data. Uses partial loop unrolling.
rdRdy       ldx    DCBPT          ; move buffer ptr from U to X
            ldd    #32*256+8      ; A = chunk count (32), B = bytes per chunk (8)
rdChnk      ldu    ,y             ; read 1st pair of bytes for the chunk
            stu    ,x             ; store to buffer
            ldu    ,y             ; bytes 3 and 4 of..
            stu    2,x            ; ..the chunk
            ldu    ,y             ; bytes 5 and 6 of..
            stu    4,x            ; ..the chunk
            ldu    ,y             ; bytes 7 and 8 of..
            stu    6,x            ; ..the chunk
            abx                   ; increment X by chunk size (8)
            deca                  ; decrement loop counter
            bne    rdChnk         ; loop if more chunks to read
*** Return Success.
            clr    CTRLATCH       ; clear carry / return controller to emulation mode
            rts                   ; restore registers and return
