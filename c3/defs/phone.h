/*
 * phone.h
 *
 * $Id: phone.h,v 1.4 2005/08/14 22:43:23 boisy Exp $
 *
 * (C) 2005 The C^3 Compiler Project
 * http://www.nitros9.org/c3/
 *
 * Notes:
 *
 * Edt/Rev  YYYY/MM/DD  Modified by
 * Comment
 * ------------------------------------------------------------------
 *          2005/08/12  Boisy G. Pitre
 * Brought in from Carl Kreider's CLIB package.
 */

#ifndef _PHONE_H
#define _PHONE_H

#define  SS_CKTK    0		/* check clock for operation */
#define  SS_CKHK    1		/* check for modem on hook */
#define  SS_ONHK    2		/* hang up phone */
#define  SS_OFHK    3		/* pick up phone */
#define  SS_TOGL    4		/* toggle squelch */
#define  SS_DIAL    5		/* dial a number */
#define  SS_DELY    6		/* delay (x) */
#define  F_PHONE 0x26		/* OS9 phone call */

#endif				/* _PHONE_H */