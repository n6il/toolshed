/*****************************************************************************
	output_os9.c	- code emitting for OS-9 modules

	Copyright (c) 2004 Chet Simpson, Digital Asphyxia. All rights reserved.

	CRC Implementation copyright 1993 Ross Williams

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
#include "proto.h"
#include "os9.h"



#define MAX_BIN_RECORD_SIZE		512
static long		binCRC = 0;
static FILE		*outputFile = NULL;
static u_int16	E_Total;					/* total # bytes for one line			*/
static u_char	E_Bytes[MAX_BIN_RECORD_SIZE];	/* Emitted held bytes					*/
FILEGEN(os9, os9, os9, true);


/* CRC Model Abstract Type */
/* ----------------------- */
/* The following type stores the context of an executing instance of the  */
/* model algorithm. Most of the fields are model parameters which must be */
/* set before the first initializing call to cm_ini.                      */
typedef struct {
	int		cm_width;   /* Parameter: Width in bits [8,32].       */
	u_int32 cm_poly;    /* Parameter: The algorithm's polynomial. */
	u_int32 cm_init;    /* Parameter: Initial register value.     */
	bool	cm_refin;   /* Parameter: Reflect input bytes?        */
	bool	cm_refot;   /* Parameter: Reflect output CRC?         */
	u_int32 cm_xorot;   /* Parameter: XOR this to output CRC.     */
	u_int32 cm_reg;     /* EnvContext: EnvContext during execution.     */
  } cm_t;

typedef cm_t *p_cm_t;




static char pbits(u_char value)
{
	char bits;
	bits = 0;

	if((value & 0x80) == 0) bits++;
	if((value & 0x40) == 0) bits++;
	if((value & 0x20) == 0) bits++;
	if((value & 0x10) == 0) bits++;
	if((value & 0x08) == 0) bits++;
	if((value & 0x04) == 0) bits++;
	if((value & 0x02) == 0) bits++;
	if((value & 0x01) == 0) bits++;

	return bits;
}

u_char os_parity(u_int16 a, u_int16 b, u_int16 c, u_char d, u_char e)
{
	return((u_char)(pbits(hibyte(a)) +
					pbits(lobyte(a)) +
					pbits(hibyte(b >> 0x08)) +
					pbits(lobyte(b & 0xff)) +
					pbits(hibyte(c >> 0x08)) +
					pbits(lobyte(c & 0xff)) +
					pbits(d) +
					pbits(e)));
}


/******************************************************************************/
/*                             Start of crcmodel.c                            */
/******************************************************************************/
/*                                                                            */
/* Author : Ross Williams (ross@guest.adelaide.edu.au.).                      */
/* Date   : 3 June 1993.                                                      */
/* Status : Public domain.                                                    */
/*                                                                            */
/* Description : This is the implementation (.c) file for the reference       */
/* implementation of the Rocksoft^tm Model CRC Algorithm. For more            */
/* information on the Rocksoft^tm Model CRC Algorithm, see the document       */
/* titled "A Painless Guide to CRC Error Detection Algorithms" by Ross        */
/* Williams (ross@guest.adelaide.edu.au.). This document is likely to be in   */
/* "ftp.adelaide.edu.au/pub/rocksoft".                                        */
/*                                                                            */
/* Note: Rocksoft is a trademark of Rocksoft Pty Ltd, Adelaide, Australia.    */
/*                                                                            */
/******************************************************************************/


/* OS-9 Module information */

/* The following definitions make the code more readable. */

#define BITMASK(X) (1L << (X))
#define MASK32 0xFFFFFFFFL


/******************************************************************************/
/* Returns the value v with the bottom b [0,32] bits reflected. */
/* Example: reflect(0x3e23L,3) == 0x3e26                        */
static u_int32 reflect(u_int32 v, int b)
{
	int   i;
	u_int32 t = v;
	for (i=0; i<b; i++) {
		if (t & 1L) {
			v |= BITMASK((b-1)-i);
		}

		else {
			v &= ~BITMASK((b-1)-i);
		}

		t>>=1;
	}
	return v;
}

/******************************************************************************/
/* Returns a longword whose value is (2^p_cm->cm_width)-1.     */
/* The trick is to do this portably (e.g. without doing <<32). */
static u_int32 widmask(p_cm_t p_cm)
{
	return (((1L<<(p_cm->cm_width-1))-1L)<<1)|1L;
}

/******************************************************************************/
/* Initializes the argument CRC model instance.          */
/* All parameter fields must be set before calling this. */
static void cm_ini(p_cm_t p_cm)
{
	p_cm->cm_reg = p_cm->cm_init;
}

/******************************************************************************/
/* Processes a single message byte [0,255]. */
static void cm_nxt(p_cm_t p_cm, int ch)
{
	int   i;
	u_int32 uch;
	u_int32 topbit;

	uch  = (u_int32) ch;
	topbit = BITMASK(p_cm->cm_width-1);

	if (p_cm->cm_refin) {
		uch = reflect(uch,8);
	}

	p_cm->cm_reg ^= (uch << (p_cm->cm_width-8));

	for(i = 0; i < 8; i++) {
		if (p_cm->cm_reg & topbit) {
			p_cm->cm_reg = (p_cm->cm_reg << 1) ^ p_cm->cm_poly;
		}

		else {
			p_cm->cm_reg <<= 1;
		}

		p_cm->cm_reg &= widmask(p_cm);
	}
}

/******************************************************************************/
/* Returns the CRC value for the message bytes processed so far. */
static u_int32 cm_crc(p_cm_t p_cm)
{
	if (p_cm->cm_refot) {
		return(p_cm->cm_xorot ^ reflect(p_cm->cm_reg,p_cm->cm_width));
	} else{ 
		return(p_cm->cm_xorot ^ p_cm->cm_reg);
	}
}


/******************************************************************************/
/*                 ctx->m_Finished of crcmodel.c                              */
/******************************************************************************/
cm_t os9_p_cm;

static void os_resetCRC(void)
{
	os9_p_cm.cm_width = 24;
	os9_p_cm.cm_poly  = 0x800063L;
	os9_p_cm.cm_init  = 0xFFFFFFL;
	os9_p_cm.cm_refin = false;
	os9_p_cm.cm_refot = false;
	os9_p_cm.cm_xorot = 0xFFFFFFL;
	cm_ini(&os9_p_cm);
}


static void addcrc(u_char val)
{
	cm_nxt(&os9_p_cm, val);
}

static u_int32 getcrc(void)
{
	return(cm_crc(&os9_p_cm));
}


void filegen_os9_flush(EnvContext *ctx, u_int16 pcreg)
{
	if(E_Total > 0) {
		fwrite(E_Bytes, 1, E_Total, outputFile);
		E_Total = 0;
	}
}

void filegen_os9_addbyte(EnvContext *ctx, u_char val, u_int16 regpc)
{
	ASSERTX(2 == ctx->m_Pass);
	ASSERTX(NULL != outputFile);
	ASSERTX(true == ctx->m_Misc.Oflag);

	E_Bytes[E_Total++] = val;
	addcrc(val);

	if(MAX_BIN_RECORD_SIZE == E_Total) {
		filegen_os9_flush(ctx, regpc);
	}

}



void filegen_os9_finish(EnvContext *ctx, u_int16 regpc)
{
	u_char u0;
	u_char u1;
	u_char u2;

	ASSERTX(2 == ctx->m_Pass);
	ASSERTX(NULL != outputFile);
	ASSERTX(true == ctx->m_Misc.Oflag);

	binCRC = getcrc();
	u0 = (u_char)((binCRC >> 16) & 0xff);
	u1 = (u_char)((binCRC >> 8) & 0xff);
	u2 = (u_char)(binCRC & 0xff);
	fprintf(outputFile, "%c%c%c", u0, u1, u2);
}



void filegen_os9_init(EnvContext *ctx, FILE *outFile)
{
	outputFile = outFile;
	binCRC = 0;
	E_Total = 0;
	os_resetCRC();
}

