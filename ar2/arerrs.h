
#ifndef lint
static char *id2 = "$Id$";
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
 * Revision 1.1  1996/07/20 17:10:40  cc
 * Initial revision
 *
 *------------------------------------------------------------------
 */


/*
 * error defns for ar/lz modules
 */

#define NOT_AR	-2					/* not an ar file						*/
#define TBLOVF	-3					/* string compression table overflow	*/
#define RERR	-4					/* I/O read error						*/
#define WERR	-5					/* I/O write error						*/
