/*
 * lowio.h
 *
 * $Id: lowio.h,v 1.4 2005/08/14 22:43:23 boisy Exp $
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

#ifndef _LOWIO_H
#define _LOWIO_H

/* path constants */
#define  STDIN       0
#define  STDOUT      1
#define  STDERR      2
/* file access constants */
#define  READ        1
#define  WRITE       2
#define  UPDATE      3
#define  DIR         0x80
#define  LOCK        -1l
#define  UNLOCK      0l
#define  TRUE        1
#define  FALSE       0
/* seek constants */
#define  FRONT       0
#define  HERE        1
#define  END         2

#endif				/* _LOWIO_H */