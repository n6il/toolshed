
#ifndef lint
static char *id3 = "$Id$";
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
 * Revision 1.1  1996/07/20 17:10:41  cc
 * Initial revision
 *
 *------------------------------------------------------------------
 */

/*
 * Definitions for library routines operating on directories.
 */

typedef struct _dirdesc {
    int     dd_fd;
    long    dd_loc;
    long    dd_size;
    char    dd_buf[BUFSIZ];
    } DIR;


#define DIRECT  struct direct

#ifndef NULL
#define NULL 0
#endif

extern DIR      *opendir();
extern DIRECT   *readdir();
extern int      closedir();

#define rewinddir(dirp)  seekdir((dirp), (long)0)
