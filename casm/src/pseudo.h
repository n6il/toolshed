/*****************************************************************************
	pesudo.h	- Declarations for pseudo ops

	Copyright (c) 2004 Chet Simpson, Digital Asphyxia. All rights reserved.

	The distribution, use, and duplication this file in source or binary form
	is restricted by an Artistic License (see license.txt) included with the
	standard distribution. If the license was not included with this package
	please refer to http://www.oarizo.com for more information.


	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that: (1) source code distributions
	retain the above copyright notice and this paragraph in its entirety, (2)
	distributions including binary code include the above copyright notice and
	this paragraph in its entirety in the documentation or other materials
	provided with the distribution, and (3) all advertising materials
	mentioning features or use of this software display the following
	acknowledgement:

		"This product includes software developed by Chet Simpson"
	
	The name of the author may be used to endorse or promote products derived
	from this software without specific priorwritten permission.

	THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR IMPLIED
	WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
	MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

*****************************************************************************/
#ifndef PSEUDO_H
#define PSEUDO_H


typedef enum {
	NULL_OP,	/* null pseudo op				*/
	PRINTDP,

	MOD,		/* Define OS-9 module header	*/
	OS9,	 	/* OS9 system call directive	*/
	VSECT,		/* Begin variable section		*/
	PSECT,		/* Begin program section		*/
	ENDSECT,	/* ctx->m_Finished section		*/

	SEGCODE,	/* Code segment					*/
	SEGDATA,	/* Data segment					*/
	SEGBSS,		/* Uninit data segment			*/
	IMPORT,		/* Import symbol				*/
	EXPORT,		/* Export symbol				*/

	EQU,		/* Equate						*/

	FILL,		/* block fill constant bytes	*/
	FCB,		/* Form Constant Bytes			*/
	FDB,		/* Form Double Bytes (words)	*/
	FQB,		/* Form quad bytes				*/
	FCC,		/* Form Constant Characters		*/
	FCS,		/* Form command string			*/
	FCZ,		/* Form null terminated string	*/
	FCN,		/* "                         "  */
	FCR,
	ZMB,		/* Zero memory bytes			*/
	ZMD,		/* Zero memory words			*/
	ZMQ,		/* Zero memory quads			*/
	RMB,		/* Reserve Memory Bytes			*/
	RMD,		/* Reserve Memory words			*/
	RMQ,		/* Reserve Memory quads			*/

	RAW,	 	/* including raw binary data	*/
	LIB,	  	/* include file into assembly	*/

	ALIGN_EVEN,	/* Align PC on even byte boundry */
	ALIGN_ODD,	/* Align PC on odd byte boundry	*/
	ALIGN,		/* Align PC on arbitrary boundry */

	NAMESPACE,	/* Begin namespace				*/
	ENDNS,		/* ctx->m_Finished namespace	*/

	UNION,		/* Begin union					*/
	ENDUNION,	/* ctx->m_Finished union		*/

	OPT, 		/* assembler option				*/
	PAGE,		/* new page						*/

	ORG,		/* Origin						*/
	END,		/* end directive				*/

	MACRO,	 	/* define macro block			*/
	ENDM,	 	/* mark end of macro block		*/

	STRUCT,		/* Struct						*/
    STRUCTDEC,	/* Structure declaration		*/
	ENDSTRUCT,	/* ctx->m_Finished of structure */

	SETDP,		/* assume DP value is this		*/
	PUSHDP,		/* Saves the current set DP val	*/
	POPDP,		/* Gets the previous DP val		*/

	IF,		 	/* if (expression != 0)			*/
	IFNDEF,		/* if !(symbol defined)			*/
	IFDEF,		/* if (symbol defined)			*/
	IFN,	 	/* if (expression != 0)			*/
	IFEQ,	 	/* if (expression = 0)			*/
	IFGT,	 	/* if (expression > 0)			*/
	IFGE,	 	/* if (expression >= 0)			*/
	IFLT,	 	/* if (expression < 0)			*/
	IFLE,	 	/* if (expression <= 0)			*/
	IFNE,	 	/* if (expression != 0)			*/
	IFP1,	 	/* if (assembler pass = 1)		*/
	IFP2,	 	/* if (assembler pass = 1)		*/
	COND,		/* if (expression != 0)			*/
	ELSE,		/* Conditional else				*/
	ENDIF,	 	/* conditional assembly			*/
	ENDC,		/* end cond statement			*/
	ENDP,    	/* end (ifp1 || ifp2)			*/

	TITLE,		/* Set title					*/
	NAME,		/* Output name					*/

	/* Macro-80c directives */
	FAIL,
	LIST,
	NLST,
	REORG,
	OP_DEX,
	OP_INX,
	CLRD_6809



} PSEUDO_OP;




#endif	/* PSEUDO_H */
