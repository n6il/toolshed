/*
 * math.h - Transcendental math functions
 *
 * $Id: math.h,v 1.3 2005/08/13 15:07:11 boisy Exp $
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

#ifndef _MATH_H
#define _MATH_H

double          acos(), asin(), atan(), sin(), cos(), tan();
double          pow(), sinh(), cosh(), tanh(), asinh(), acosh(), atanh();
double          exp(), antilg(), log10(), log();
double          trunc(), sqrt(), sqr(), inv();
double          dexp(), dabs();
int             rad(), deg();

#endif				/* _MATH_H */
