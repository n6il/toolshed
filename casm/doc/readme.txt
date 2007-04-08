

                            General Release Notes
                            ---------------------

Version 3.0 is an update to the current release of CASM. It includes several
new features as well as many updates in order to provide compatibility with
source code written for other assemblers.

In addition to the updates several bugs related to the parsing of source
files have also been made. This includes bitwise memory operations,
register to register operations, and fixes to the parsing and recognition
of indirect addressing usage.

Three new output file formats have been included with this release. CASM
now generates ROM based files, Raw binary executables, and an experimental
form of shared libraries called loadable modules.

Many changes have also been made in order to provide compatibility with source
code originally written for EDTASM6309 by Robert Gault and CCASM by Roger
Taylor. All opcode formats and pseudo mneumonics are now supported with the
exception of unions and namespaces available in CCASM. In order to provide
support for those features I will be rewriting the code responsible for
managing symbol tables, structures, and macros in order to provide better
support for scoped data and code access. Although both namespaces and
unions could be added to the current code base it would not be pretty
and would most likely be problematic.

The following is a list of mneumonics added to support other assemblers:
	
	includebin	align	even	odd	cond	ifp2	endc	endp
	include		fcn	fcr	fcz	fzb	fdq	fzq	rmd
	endstruct	rmq	rzb	rzd	rzq	title	zmb	zmd
	zmq		copy	copy-	imp	exp	tfrp	tfrm	tfrs
	tfrr


Not all instructions are documented here but in most cases the 6809
instructions follow the format of edtasm and 6309 instructions follow the
syntax outlines in the HD6309 Technical Reference Manual. Both this
document and the HD6309 Technical Reference Manual are being updated and
merged into a single document. This new document will is being written in
a similar format to that of the 6809 Assembly Language Programming book
by William Barden Jr., a popular programming book for the Color Computer.



                         Updates to supported expressions
                         --------------------------------

The code responsible of evaluation expressions has been completely revamped
with several additions made in order to provide compatibility with other
assemblers. The following is a quick list of updates to the accepted
expression syntax:


	Operators:
		
		+	add
		-	subtract
		*	multiply
		/	divide
		%	modulus
		&	bitwise and
		|	bitwise or
		^	bitwise exclusive-or
		<<	Shift value left
		>>	Shift value right
		&&	Logical and
		||	Logical or
		
	Conditional operators
		=	equals
		<>	not equals
		<	less than
		>	greater than
		<=	less than or equals
		>=	grater than or equals
		

	Constant values
                '	Single ASCII character
                $	hexadecimal constant
                !	octal constant
                %	binary constant
                digit	decimal constant


Expressions are evaluated from left to right with no provision for operator
precedence as with languages such as C. This version of CASM does however
support subexpressions enclosed in parenthesis. Evaluation of sub expressions
is performed at the deepest level of expression first and carried outwards
through all upper level expressions.

The most prominent additions to V3.0 is the conditionals which are supported
by Roger Taylows CCASM and the binary shift operators ("<<" and ">>") which
are specific to CASM V3.0.


The following is a BNF notation for the expression syntax:

	operator ::=	'+' |
			'-' |
			'*' |
			'/' |
			'|' |
			'&' |
			'%' |
			'^' |
			'=' |
			'<' |
			'>' |
			'<>' |
			'<=' |
			'>=' |
			'<<' |
			'>>' |
			'&&' |
			'||'
 
       expr ::=  <term> | <expr> <operator> <expr> | "(" <expr> ")"

       term ::=  <symbol> | '*' | <constant>
 
       symbol ::=  [ "@" ] "a..z,_" { "0..9,a..z,_,$,@" } [ "." <symbol> ]
 
       constant ::= <hex_constant> |
                    <binary_constant> |
                    <octal_constant> |
                    <decimal_constant> |
                    <ascii_constant>
 
       hex_constant ::= "$" "0..9,a..f" { "0..9,a..f" }
 
       octal_constant ::= "!" "0..7" { "0..7" }
 
       binary_constant ::= "%" "0..1" { "0..1" }
 
       decimal_constant ::= "0..9" { "0..9" }
 
       ascii_constant ::= '<any printing char>
 



                            Command line options
                            --------------------

All command line options begin with a dash (-) and unlike earlier versions
of the assembler can appear before, after or inbetween the source filenames.



General CASM options:
---------------------
-help                   Displays this list
-?                      Displays this list
-6809                   Allow only 6809 Opcodes
-D                      Define symbol
-C                      Use case sensative labels
-silent                 Run in silent mode
--no-warn               Disable warnings

Output file options:
--------------------
-O                      Set output directory
-o                      Set output name
-bin                    Generate CoCo RS-DOS binary file
-s19                    Generate S-record file
-rom                    Generate a padded ROM file
-raw                    Generate raw binary file
-mod                    Generate shared object module
-os9                    Generate Microware OS-9 module
-rof                    Generate Microware Relocatable Object File
-obj                    Generate CASM object file
-noout                  Assemble only, do not generate output file

Compatibilty options:
---------------------
--mode-rma              Assemble in RMA compatibility mode
--mode-macro80c         Assemble in Macro-80c compatibility mode
--mode-edtasm           Assemble in EDTASM+ compatibility mode
--mode-edtasm6309       Assemble in EDTASM6309 compatibility mode
--mode-ccasm            Assemble in CCASM compatibility mode
--warn-portable         Warn of source code portability issues
--force-zero-offset     Enable explicit 0 index offset
--force-pc-relative     Use relative offsets for both PC and PCR
--disable-macros        Disable use of macros
--disable-locals        Disable use of local labels
--strict-locals         Enable use of strict local label rules
--ignore-case           Ignore case on symbols
--enable-precedence     Enable operator precedence in expressions

Assembler listing options:
--------------------------
-list                   Generate formatted source file listing to stdout
-nolist                 Source output disabled
-cycle                  Cycle count enabled
-slist                  Generate formatted list of labels
-xref                   Cross refence table disabled
-expand                 Macro definitions will be expanded in listing
-opt                    Display number of lines that could be optmized
-alert                  Display source lines that could be displayed
-noln                   Do not print line numbers in listing
-noopdata               Do not print opcode data in listing
-cm                     Comment out macros in listing




                       Generating Loadable Modules
                       ---------------------------

Loadable modules are similar to shared libraries on present day Operating
Systems. When creating loadable modules developers should take care to either
write position independant code or to specify the starting MMU block and
address the module should be loaded to.

When a loadable module is specified as the output type some behavour of the
assembler is modified. The following cases should be taken into consideration
when creating a loadable module:


	1. The ORG directive accepts both an MMU address and PC (Program
	   Counter) address the following form:
		
			org	{expression},{expression}

	   The first expression specifies the MMU block the module should
	   be loaded into and the second expression is the address of the
	   Program Counter at the first executable instruction.
	   
	   The 'org' statement can only be used ONCE during the assembly
	   process and should be the first statement prior to any code
	   being assembled.
	   
	2. Each loadable module must include a 'modinit' label somewhere
	   in the source file. This is used as the base entry point for
	   the module. If a 'modinit' label is not provided the assembly
	   will fail. The following is an example of using 'modinit'.
	   
	   
		modinit	jsr	initrnd
			rts

	   Presently no entry conditions have been defined for modinit.
	   It is currently up to the developer to define these conditions.
	   

The usage and specifics of the loabable module have not yet been defined.
The final version of CASM 3.0 will clearly define these conditions and
usage in order to create a standard of the use of loadable modules.












                     Data Related Pseudo Mneumonics
                     ------------------------------

=, SET, EQU
------------

	Defines a numeric constant.

		format:
			label	equ   {expression}

		examples:
			mode	equ	$10
			mode	equ	5
			mode	set	1
			mode	=	3



FCB - Form Character Byte(s)
-----------------------------

	Defines a data field made up of specified bytes.

		format:
			fcb   {expression}[,{expression},{expression},...]

		examples:
			fcb   10
			fcb   $10,$10,50,$20

	FCB is special in that it provides compatibility with the Macro-80c
	specific declaration of repeating sections. Repeating sections are
	available whether Macro-80c compatibility is enabled or not.
	
		format:
			fcb   {expression}[{expression},{expression},...]
			
		examples:
			fcb   10[$33]	* Repeate $33 10 times
			fcb   10[$3,$5]	* Repeate $3,$5 10 times



FDB - Form Double Byte(s)
--------------------------

	Defines a data field made up of double bytes (16 bit values).

		format:
			fdb   {expression}[,{expression},{expression},...]

		examples:
			fdb	510
			fdb	0,$500,100



FQB - Form Quad Byte(s)
------------------------

	Defines a data field made up of quad bytes (32 bit values).

		format:
			fqb   {expression}[,{expression},{expression},...]

		examples:
			fdb	510
			fdb	$eeffddcc,$500,100



FCC - Form Character String
----------------------------

	Defines a data field made up of a text string. An fcc is special that 
	the first character of the operand becomes the delimiter. The
	assembler assumes the string to terminate when the delimeter is
	encountered again.

		format:
			fcc	"string"

		examples:
			fcc	"this is nice"
			fcc	'this is normal'



FCS - Form Character Set
-------------------------

	Forms command set.  This works exactly like fcc except that the last
	character in the string has its high bit set.



FCN, FCZ - Form null ternimated character byte string
------------------------------------------------------

	Defines a string that is automatically terminated by a 0 byte value. This
	works exactly like fcc does, except that a null or zero value byte is
	automatically placed after the last character of the string.



FCR - Form carriage return/null terminated byte string
-------------------------------------------------------

	Defines a string that is automatically terminated by a carriage return and
	null byte combination. This works exactly like fcc does, except that a
	carriage return byte folloed by a zero value byte is automatically placed
	after the last character of the string.



RMB - Reserve Memory Bytes
---------------------------

	Reserve X number of bytes. This allows you to reserve (skip) a certain number
	of bytes without actually generating and data in the final output file.

		format:
			rmb	{expression}

		examples:
			rmb	1	* Reserves 1 byte
			rmb	500	* Reserves 500 bytes



RMD - Reserve Memory Double Bytes
----------------------------------

	Reserve X number of double bytes. This allows you to reserve (skip) a certain
	number of double bytes without actually generating and data in the final
	output file.

		format:
			rmd   {expression}

		examples:
			rmd	1	* Reserves 2 bytes of memory
			rmd	500	* Reserves 1000 bytes of memory


RMQ - Reserve Memory Quad Bytes
--------------------------------

	Reserve X number of quad bytes. This allows you to reserve (skip) a certain
	number of quad bytes without actually generating and data in the final
	output file.

		format:
			rmd	{expression}

		examples:
			rmd	1	* Reserves 4 bytes of memory
			rmd	500	* Reserves 2000 bytes of memory



FILL - Fill memory with value
------------------------------

	This is similar to rmb, and acts almost exactly alike except that
	it generates a the same data byte a specified number of times.

		format:
			fill	data,count

		examples:
			fill	0,500	* Fills 500 bytes with value 0
			fill	5,30	* Fills 30 bytes with value 5



BSZ, FZB, RZB, ZMB
-------------------

	These are similar to rmb, and act almost exactly alike except that
	they generate byte value in memory a specified number of times.

		format:
			pseudo_op  {expression}

		example:
			zmb	500	* Fills 500 bytes with value 0



FZD, RZD, ZMD
--------------

	These are similar to rmd, and act almost exactly alike except that
	they generate double byte values in memory a specified number of times.

		format:
			pseudo_op  {expression}

		example:
			fzd	1
			zmd	500	* Fills 1000 bytes with value 0



FZQ, RZQ, ZMQ
--------------

	These are similar to rmd, and act almost exactly alike except that
	they generate quad byte values in memory a specified number of times.

		format:
			pseudo_op  {expression}

		example:
			fzq	1	* Fills 4 bytes with value 0
			zmq	500	* Fills 2000 bytes with value 0








                         External file pseudo mneumonics
                         -------------------------------



USE, LIB, INCLUDE
------------------

	Includes an external source file. Both have been included in
	in order to support several older assemblers.



RAW, INCLUDEBIN
----------------
	Links in any file, regaurdless of type, starting at that location.
	Handy for including binary data instead of converting it to fcb
	statements and then assembling it.




                     Conditioanl assembly pseudo mneumonics
                     --------------------------------------

Conditinal assembly allows you to specify different routines to be assembled
at different times.  This is useful when writing routines that must be run
on several machines that all have different hardware. Conditional assembly
statements can be nested for several depths.

COND		- If the expression is non-zero, the code following the
		  statement is assembled until either an 'else', 'endc' or
		  other conditional assemby mneumonic is encountered. This
		  statement must be terminated by a matching 'endc' directive.

IF, IFNE	- If the expression is non-zero, the code following the
		  statement is assembled until either an 'else', 'endc' or
		  other conditional assemby mneumonic is encountered. This
		  statement must be terminated by a matching 'endc' directive.

IFP1		- If in pass 1 of the assembly, the code following the
		  statement is assembled until an 'else', 'endp' or other
		  contional is reached. This conditional must be terminated
		  with 'endp'.

IFP2		- If in pass 1 of the assembly, the code following the
		  statement is assembled until an 'else', 'endp' or other
		  contional is reached. This conditional must be terminated
		  with 'endp'.

IFN, IFEQ	- If the expression is zero, the code following the statement
		  is assembled until either an 'else', 'endc' or other
		  conditional assemby mneumonic is encountered. This statement
		  must be terminated by a matching 'endif' directive.

IFLT		- If the expression is less than zero, the code following the
		  statement is assembled until either an 'else', 'endc' or
		  other conditional assemby mneumonic is encountered. This
		  statement must be terminated by a matching 'endif'
		  directive.

IFLE		- If the expression is less than or equal to zero, the code
		  following the statement is assembled until either an 'else',
		  'endc' or other conditional assemby mneumonic is
		  encountered. This statement must be terminated by a matching
		  'endif' directive.

IFGT		- If the expression is greater than zero, the code following the
		  statement is assembled until either an 'else', 'endc' or
		  other conditional assemby mneumonic is encountered. This
		  statement must be terminated by a matching 'endif'
		  directive.

IFGE		- If the expression is greater than or equal to zero, the code
		  following the statement is assembled until either an 'else',
		  'endc' or other conditional assemby mneumonic is
		  encountered. This statement must be terminated by a matching
		  'endif' directive.
		  
IFDEF		- If the symbol is defined the code following the conditional
		  statement will is assembled until either an 'else', 'endif'
		  or other conditional assemby mneumonic is encountered. This
		  statement must be terminated by a matching 'endif'
		  directive.
		  
IFNDEF		- If the symbol is not defined the code following the
		  conditional statement will is assembled until either an
		  'else', 'endif' or other conditional assemby mneumonic is
		  encountered. This statement must be terminated by a
		  matching 'endif' directive.

ELSE		- Conditional flip-flop.

ENDIF		- Ends conditional assembly.

ENDP		- Terminates a conditional statement started with IFP1 or IFP2.

ENDC		- Terminates a conditional statement started with COND.



			    Code Alignment Pseudo Mneumonics
                            --------------------------------


ALIGN - Align Program Counter on a specified byte boundry
----------------------------------------------------------

	When the assembler encounters this directive the current internal
	program counter used for code generation will be aligned to the
	specified byte boundry. For generated binaries that do not support the
	logical division of code sections that do not have any data or code in
	between sections zero value bytes will be generated.

		Format:

			align	{expression}

		Example:
		
			align	4	* Align on 4 byte boundry
			align	8	* Align on 8 byte boundry
			align	23	* Align on 23 byte boundry



EVEN - Align Program Counter on an even byte boundry
-----------------------------------------------------

	When the assembler encounters this directive the current internal
	program counter used for code generation will be aligned to an even
	byte boundry. For generated binaries that do not support the logical
	division of code sections that do not have any data or code in between
	sections zero value bytes will be generated.



ODD - Align Program Counter on an odd byte boundry
---------------------------------------------------

	When the assembler encounters this directive the current internal
	program counter used for code generation will be aligned to an odd
	byte boundry. For generated binaries that do not support the logical
	division of code sections that do not have any data or code in between
	sections zero value bytes will be generated.


		                  Namespace directives
				  --------------------
				  

		            Structure and Union Declarations
			    --------------------------------


STRUC, STRUCT - Declare Structure
ENDS, ENDSTRUCT - End Structure Declaration
----------------------------------

	Begins a structure declaration. Structure declarations are terminated
	with either ENDS or ENDSTRUCT. Mneumonics which generate code or data
	cannot be used inside of a struct declaration. 

		Format:

			label	struct
				{data declarations}
				ends

		Example:
			coord	struct		* Declare a coordinate structure
			x	rmb	2	* X coordinate
			y	rmb	2	* Y coordinate
				ends

			static	coord

				lda	coord.x,u
				lda	$400+coord.x
				lda	static.x
				ldx	#static
				lda	coord.x,x





			         OS-9 Pseudo Mneumonics
                                 ----------------------


OS9 - Generate OS-9 System call
--------------------------------

	Creates code for OS-9 Level I/II system call
	


MOD - Generates OS-9 module header
-----------------------------------

	Creates an OS-9 module header at current location in the file

psect
vsect
endsect



			        Code Generation Directives
				--------------------------

.bss

.code

.data

.export

.import
.extern
.extrn

EXPORT - Export symbol name
----------------------------

	When generating a loadable module binary (see -mod option) the export
	directive allows you to make the address of a label publicly
	accessible in the module. For more information see the "Loadable
	Modules" section.
	
		Format:

				export	label

		Example:

				export	getname
			name	fcn	"Module Name"
			getname	leax	name,pcr
				rts





				Local and global labels
				-----------------------

Global labels can be up to 31 characters in length, must start with a letter
or underscore, and can contain letters, numbers, underscores, dollar signs,
and periods.

In order to maintain compatibility with other assemblers CASM accepts a
variety for local label formats. In order to declare a label as local it must
include either an "at" symbol or question mark. Sections of code using local
labels are separated by blank lines in the source code. This distinguishes
the usage local labels using the same name.

	Examples:
			as9_is_cool	rts

					ldb	#$10
			@a		decb
					bne	@a

			a@		decb
					bne	a@
					
			a?a		decb
					bne	a?a


				Macro pseudo mneumonics
				-----------------------

Macros support a wide range of features including internal labels and parameters.
All internal labels must start with an 'at' symbol as they are all local to the
macro. All parameters that are passed to the macro must be accessed by using the
parameter label (specified in the macro definition) enclosed in curly braces.


MACRO - Begin MACRO declaration
ENDM - End MACRO declaration
--------------------------------

	Respectively defines and ends a macro with paramters

		examples:
			idata    macro
				 lda   #$100
				 sta   ,x++
				 sta   ,x
				 com   ,x++
				 endm

			startup  ldx   #data
				 idata
				 ldx   #term
				 idata
				 idata
				 idata
				 rts

			doit	macro	arg1,arg2,arg3
				lda	#{arg1}
				sta	{arg2}
				ldb	{arg3}
			@a	lsla
				decb
				bne	@a




			      Direct page pseudo mneumonics
			      -----------------------------

SETDP - Sets the internal direct page value
--------------------------------------------

	Informs the compiler of the direct page value.  Used to determine if
	an opcode should be generated in direct or extended mode.
	
		Format:
		
			setdp	{expression}

		Examples:

			setdp	$00
			setdp	1-label



PUSHDP - Save the internal Direct Page value
---------------------------------------------

	Saves the current Direct Page value onto an internal stack in the assembler.
	This allows the value to be retrieved later on during the assembler process.
	CASM has an internal Direct Page stack limit of 255.

		Format:
		
			pushdp


POPDP - Restore the internal Direct Page Value
-----------------------------------------------

	Restores a previously saved Direct Page value.


		Format:
		
			popdp

		Example:

			setdp	$01	* Sets internal DP to $01
			pushdp		* Saves the internal DP
			setdp	$02	* Sets internal DP to $02
			lda	$dd	* Loads value in $2dd into ACCA
			popdp		* Restores the DP to $01
			lda	$ee	* Loads value in $1ee into ACCA




			Output Listing Pseudo Mneumonics
			--------------------------------


NAM, NAME - Sets the output filename
-------------------------------------

	This option is not currently implemented and is provided for historical
	compatibility.


TTL, TITLE - Set the title of the output listing
-------------------------------------------------

	This option is not currently implemented and is provided for historical
	compatibility.



SPC - Outputs blank lines to the output listing
------------------------------------------------

	This option is not currently implemented and is provided for historical
	compatibility.



PAG, PAGE
----------

	Outputs a page break in the source listing.







				General pseudo mneumonics
				-------------------------

ORG - Set assembly origin
--------------------------

   	Sets the origin for the assembler file.  Multiples are allowed to
   	changed the positioning of code during assembly.

   		Format:
   			org	{expression}


   		Example:

	   		org	$e00
			org	start


END - Indicates the end of assembly
------------------------------------

	Inidicates end of assembly, and optionally the execution entry
	address.

		Format:

			end	{expression}

		Example:

			end	start



OPT - Set or clear an internal assembler option
------------------------------------------------

	Sets internal assembler options. The supported options available
		are:
			l	enable output listing.
			nol	disable output listing (default).
			cre	generate cross reference table.
			s	generate a symbol table.
			c	enable cycle count.
			noc	disable cycle count.
			contc	continue counting cycles

