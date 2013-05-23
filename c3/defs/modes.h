/*
 * modes.h - File modes and permissions expressed in 8 bits
 *
 * $Id: modes.h,v 1.3 2005/08/13 15:07:11 boisy Exp $
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

#ifndef _MODES_H
#define _MODES_H

/* file modes */
#define S_IFMT     0xff		/* mask for type of file */
#define S_IFDIR    0x80		/* directory */

/* permissions */
#define  S_IPRM    0xff		/* mask for permission bits */
#define  S_IREAD   0x01		/* owner read */
#define  S_IWRITE  0x02		/* owner write */
#define  S_IEXEC   0x04		/* owner execute */
#define  S_IOREAD  0x08		/* public read */
#define  S_IOWRITE 0x10		/* public write */
#define  S_IOEXEC  0x20		/* public execute */
#define  S_ISHARE  0x40		/* sharable */

#endif				/* _MODES_H */
