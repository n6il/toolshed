/*****************************************************************************
	proc_util.c	- OPcode processing utils

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
#include "as.h"
#include "cpu.h"

/*
 *	  GetRegister --- return register number of *line->m_Ptr
 */
REGISTER GetRegister(EnvContext *line, const bool allowZeroText, const bool allowZeroReg)
{
	if(head(line->m_Ptr,"d")) {
		line->m_Ptr++;
		return(REG_ACCD);
	}

	if(head(line->m_Ptr,"x")) {
		line->m_Ptr++;
		return(REG_INDX);
	}

	if(head(line->m_Ptr,"y")) {
		line->m_Ptr++;
		return(REG_INDY);
	}

	if(head(line->m_Ptr,"u")) {
		line->m_Ptr++;
		return(REG_USTACK);
	}

	if(head(line->m_Ptr,"s")) {
		line->m_Ptr++;
		return(REG_SSTACK);
	}

	if(head(line->m_Ptr,"pc")) {
		line->m_Ptr += 2;
		return(REG_PC);
	}


	if(head(line->m_Ptr,"w")) {
		line->m_Ptr++;
		return(REG_ACCW);		/* 6309 register */
	}

	if(head(line->m_Ptr,"v")) {
		line->m_Ptr++;
		return(REG_V);			/* 6309 register */
	}

	if(head(line->m_Ptr,"a")) {
		line->m_Ptr++;
		return(REG_ACCA);
	}

	if(head(line->m_Ptr,"b")) {
		line->m_Ptr++;
		return(REG_ACCB);
	}

	if(head(line->m_Ptr,"cc")) {
		line->m_Ptr += 2;
		return(REG_CC);
	}

	if(head(line->m_Ptr,"dp")) {
		line->m_Ptr += 2;
		return(REG_DP);
	}

	if(true == allowZeroReg) {
		if(true == allowZeroText && head(line->m_Ptr, "zero")) {
			line->m_Ptr += 4;
			return REG_ZERO;
		}

		if(head(line->m_Ptr,"o")) {
			line->m_Ptr++;
			return(REG_ZERO);		/* 6309 register */
		}

		if(head(line->m_Ptr,"0")) {
			line->m_Ptr++;
			return(REG_ZERO);		/* 6309 register */
		}

		if(head(line->m_Ptr,"00")) {
			line->m_Ptr += 2;
			return(REG_ZEROZERO);		/* 6309 register */
		}

	}

	if(head(line->m_Ptr,"e")) {
		line->m_Ptr++;
		return(REG_ACCE);		/* 6309 register */
	}

	if(head(line->m_Ptr,"f")) {
		line->m_Ptr++;
		return(REG_ACCF);		/* 6309 register */
	}

	if(head(line->m_Ptr,"pcr")) {
		line->m_Ptr += 3;
		return(REG_PCR);
	}

	return(REG_ERROR);
}



const char *GetRegisterName(const REGISTER r)
{
#define DOREG(x, y)	case x: return y;
	switch(r) {
	DOREG(REG_ACCD, "d");
	DOREG(REG_INDX, "x");
	DOREG(REG_INDY, "y");
	DOREG(REG_USTACK, "u");
	DOREG(REG_SSTACK, "s");
	DOREG(REG_PC, "pc");
	DOREG(REG_ACCW, "w");		/* 6309 register */
	DOREG(REG_V, "v");			/* 6309 register */
	DOREG(REG_ACCA, "a");
	DOREG(REG_ACCB, "b");
	DOREG(REG_CC, "cc");
	DOREG(REG_DP, "dp");
	DOREG(REG_ZERO, "0");		/* 6309 register */
	DOREG(REG_ZEROZERO, "00");	/* 6309 register */
	DOREG(REG_ACCE, "e");		/* 6309 register */
	DOREG(REG_ACCF, "f");		/* 6309 register */
	DOREG(REG_PCR, "pcr");
	default:
		return "unknown";
	}
#undef DOREG
}



/*
 *	  GetAddrMode --- determine addressing mode from operand field
 */

#define IS_STACK(a)		(reg == REG_SSTACK || reg == REG_USTACK)
#define IS_INDEX(a)		(reg == REG_INDX || reg == REG_INDY)
#define IS_PC(a)		(reg == REG_PC || reg == REG_PCR)

ADDR_MODE GetAddrMode(EnvContext *line)
{
	const char *p;
	REGISTER reg;

	if( *line->m_Operand  == '#' ) {
		return(IMMEDIATE);		  /* immediate addressing */
	}

	if('[' == *line->m_Operand) {
		return INDIRECT;
	}

	p = line->m_Operand;

	/* Check for comma */
	while(false == IsWS(*p) && EOS != *p) {
		if(*p == ',') {
			return(INDEXED);	/* indexed addressing */
		}

		p++;
	}

	p = line->m_Operand;

	if(*p == '>') {
		return(EXTENDED);
	}

	if(*p == '<' ) {
		return(DIRECT);
	}

	if( *p == '[' ) {
		p++;
	}

	/* Skip index decrement */
	while(*p == '-') {
		p++;
	}

	reg = GetRegister(line, false, false);

	/* indexed addressing */
	if(IS_STACK(reg) || IS_INDEX(reg) || IS_PC(reg) ) {
		return(INDEXED);
	}

	 /* Sanity check for indirect addressing */
	if( *line->m_Operand == '[') {
		return(INDIRECT);
	}

	return(EXTENDED);
}


