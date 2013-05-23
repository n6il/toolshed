/*
 * stdlib.h
 *
 * $Id: stdlib.h,v 1.7 2005/12/02 08:09:32 tlindner Exp $
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
 *
 *          2005/08/14  Boisy G. Pitre
 * stdlib.h now includes types.h so that standard types are pulled
 * in automatically.
 */

#ifndef _STDLIB_H
#define	_STDLIB_H

/* Exclude this until there is compiler support
#include		<types.h>
*/

#define	EXIT_FAILURE	1
#define	EXIT_SUCCESS	0

int				exit();
double          atof();
int             atoi();
long            atol();
char           *itoa();
char           *ltoa();
char           *utoa();
int             htoi();
long            htol();
int             max();
int             min();
unsigned        umin();
unsigned        umax();
char           *calloc();
char           *malloc();
char           *realloc();

/*
void     free();
*/

#endif				/* _STDLIB_H */
