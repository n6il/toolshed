
#ifndef lint
static char *id6 = "$Id$";
#endif

/*
 *------------------------------------------------------------------
 *
 * $Source$
 * $RCSfile$
 * $Revision$
 * $Date$
 * $State$
 * $Author$
 * $Locker$
 *
 *------------------------------------------------------------------
 *
 * Carl Kreider (71076.76@compuserve.com, crkreider@delphi.com)
 * Syscon International Inc
 * 1108 S. High Street
 * South Bend, IN  46601-3796
 * (219) 232-3900
 *
 *------------------------------------------------------------------
 * $Log$
 * Revision 1.1  1996/07/20 17:10:43  cc
 * Initial revision
 *
 *------------------------------------------------------------------
 */

#include <sys/types.h>
#include <sys/stat.h>

#define S_IGREAD		00040
#define S_IGWRITE		00020
#define S_IGEXEC		00010
#define S_IOREAD		00004
#define S_IOWRITE		00002
#define S_IOEXEC		00001

#define GROUP_BITS		(S_IGREAD|S_IGWRITE|S_IGEXEC)
#define CATEGORY_BITS	(S_IFMT|S_IFDIR)
#define SPECIAL_BITS	(S_IFCHR|S_IFBLK|S_IFREG|S_IFIFO)
#define CUSTOM_BITS		(S_ISUID|S_ISGID|S_ISVTX)
#define NON_OS9_FLAGS	(GROUP_BITS|CATEGORY_BITS|SPECIAL_BITS|CUSTOM_BITS)
#define NON_UNIX_FLAGS	(0x80|0x40)
#define CHARACTER_OF(C)	((char) ((C) & 0xff))

extern char		*u2oDate();
extern long		o2uDate();
extern char		u2oFmode();
extern short	o2uFmode();
