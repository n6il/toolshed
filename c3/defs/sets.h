/*
 * sets.h
 *
 * $Id: sets.h,v 1.3 2005/08/13 15:07:11 boisy Exp $
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

#ifndef _SETS_H
#define _SETS_H

#define SETMAX   255		/* ie 0..255 */
char           *allocset();
char           *addc2set(s, c);
char           *adds2set(s, p);
char           *rmfmset(s, c);
int             smember(s, c);
char           *sunion(s1, s2);
char           *sintersect(s1, s2);
char           *sdifference(s1, s2);
char           *copyset(s1, s2);
char           *dupset(s);

#endif				/* _SETS_H */
