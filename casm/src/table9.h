/*****************************************************************************
	table9.h	- Declarations for opcodes

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
#ifndef TABLE9_H
#define TABLE9_H


/* an entry in the mnemonic table */
#include "config.h"
#include "cpu.h"


typedef struct {
	int		m_Immediate;
	int		m_Direct;
	int		m_Indexed;
	int		m_Extended;
	int		m_Inherent;
} CycleCount;

typedef struct {
	u_int16		compatMask;		/* The level this mneumonic is allowed */
	CPUTYPE		cputype;		/* CPU type (6809/6309) */
	char		*mnemonic;		/* its name */
	u_char		immsize;		/* Datasize for immediate addressing mode */
	char		optype;			/* its class */
	u_char		page;			/* opcode page */
	u_char		opcode;			/* its base opcode */
	CycleCount	cycles6809;		/* its base # of cycles */
	CycleCount	cycles6309;		/* its base # of cycles */
} Mneumonic;

void InitOpcodeTable();

const Mneumonic *mne_look(EnvContext *ctx, const char *str);
int GetCycleCount(EnvContext *ctx, const Mneumonic *op, const ADDR_MODE mode, const CPUTYPE cpu);


#endif	/* TABLE9_H */
