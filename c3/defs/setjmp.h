/*
 * setjmp.h
 *
 * $Id: setjmp.h,v 1.3 2005/08/13 15:07:11 boisy Exp $
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

#ifndef _SETJMP_H
#define _SETJMP_H

#ifdef OSK
typedef int     jmp_buf[16];

#else
typedef int     jmp_buf[4];

#endif

#endif				/* _SETJMP_H */
