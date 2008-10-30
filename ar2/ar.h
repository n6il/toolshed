
#ifndef lint
static char *id1 = "$Id$";
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
 * Revision 1.6  2008/10/30 03:08:48  boisy
 * Additional updates
 *
 * Revision 1.5  2007/10/06 06:27:24  tlindner
 * Updated for CYGWIN, Should not have broken anything. :)
 *
 * ----------------------------------------------------------------------
 *
 * Revision 1.4  2006/09/09 01:59:03  boisy
 * Changes to accomodate compiling under Turbo C++
 *
 * Revision 1.3  2006/04/11 01:32:45  boisy
 * Fixed warnings under Linux
 *
 * Revision 1.2  1996/07/20 22:15:58  cc
 * Merged in pwz's unixification (Sunos).
 *
 * Revision 1.1  96/07/20  17:10:39  cc
 * Initial revision
 * 
 *------------------------------------------------------------------
 */

#ifdef NOSTRCHR
# define  strchr  index
# define  strrchr rindex
#endif

#ifdef SYSV
# define  strucmp strcmp
#endif

#include <errno.h>

/* extern int   errno; */

#include "arerrs.h"

#ifdef MSDOS
#define  F_RP    "r+b"
#define  F_WP    "w+b"
#define  F_R     "rb"
#define  F_W     "wb"
#else
#define  F_RP    "r+"
#define  F_WP    "w+"
#define  F_R     "r"
#define  F_W     "w"
#endif

#ifndef SEEK_SET
# define SEEK_SET	0
#endif
#ifndef SEEK_CUR
# define SEEK_CUR	1
#endif

#define FALSE	(0)
#define TRUE	(FALSE == 0)
#define ERROR	(-1)

#define HID		"+AR0.0+"
#define HIDSIZ	7
#define SUF		".ar"
#define SUFSIZ	3
#define FNSIZ	65
#define MAXLINE	256
 
#define PLAIN	0					/* plain text or object			*/
#define SQ		1					/* old fashion CPM squeeze		*/
#define COMP1	2					/* LZ compression 9..11 bits	*/
#define COMP2	3					/* obsolete, not used			*/
#define COMP3	4					/* LZ comperssion 9..13 bits	*/


typedef struct {						/* obvious definitions		*/
	char	fd_attr;
	char	fd_own[2];
	char	fd_date[5];
	char	fd_link;
	char	fd_fsize[4];
	char	fd_dcr[3];
	} FILDES;


/* NOTE that a_size is on an even byte boundary */
typedef struct {
	char	a_hid[HIDSIZ+1];		/* header id string				*/
	char	a_name[FNSIZ+1];		/* name of the archived file	*/
	long	a_size;			/* size of archive (not virgin) file	*/
	char	a_type;			/* archive type - virg, packed, etc		*/
	char	a_stat;			/* status of file - good, deleted, ..	*/
	FILDES	a_attr;			/* attributes of the archived file		*/
	} HEADER;

/* since comilers for big machines want to pad HEADER, we hack	*/
#define SIZEOF_HEADER	96L

typedef struct fn {
	struct fn	*fn_link;				/* link to next file name	*/
	char		fn_name[1];				/* the name itself			*/
	} FN;
