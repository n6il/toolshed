/*****************************************************************************
	cpu.c	- Implementation for 6809/6309 CPU specifics

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
#include "cpu.h"
#include "error.h"
#include "util.h"

#define MAX_DP_STACK	256

static u_int16	regtenPC;                  	/* Program Counter              		*/
static u_int16	regtenOldPC;              	/* Program Counter at beginning 		*/
static u_int16	regtenDP;				/* storage to DP contents assumption 	*/
static u_char	dpStack[MAX_DP_STACK];
static int		dpStackPtr = 0;


void SetDPReg(const u_char val)
{
	regtenDP = (val & 0xff);
}


void InitCPU(void)
{
	SetPCReg(0);
	SetOldPCReg(0);
	regtenDP = 0x0000;
	dpStackPtr = 0;
}


u_char GetDPReg(void)
{
	return lobyte(regtenDP);
}

void SetPCReg(const u_int16 val)
{
	regtenPC = val;
}

void SetOldPCReg(const u_int16 val)
{
	regtenOldPC = val;
}

u_int16 GetPCReg(void)
{
	return regtenPC;
}


u_int16 GetOldPCReg(void)
{
	return regtenOldPC;
}

u_int16 BumpPCReg(const u_int16 val)
{
	regtenPC = regtenPC + val;
	return regtenPC;
}

void PushDPReg(EnvContext *ctx)
{
	if(dpStackPtr >= MAX_DP_STACK) {
		error(ctx, ERR_GENERAL, "too many Direct Page values saved");
	} else {
		dpStack[dpStackPtr] = (u_char)GetDPReg();
	}

	dpStackPtr++;
}

void PopDPReg(EnvContext *ctx)
{
	if(0 == dpStackPtr) {
		warning(ctx, WARN_DPSTACK, "no DP value has been saved : value of DP remains unchanged");
	} else {

		/* Restore DP only if it's within the stack */
		if(dpStackPtr < MAX_DP_STACK) {
			SetDPReg(dpStack[dpStackPtr]);
		}

		dpStackPtr--;
	}
}


bool IsAddressInDirectPage(const u_int16 addr)
{
	return(hibyte(addr) == GetDPReg());
}
