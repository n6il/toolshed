#include <stdlib.h>
#include <rof.h>
#include "out.h"

#define DEBUG

typedef unsigned u16;		/* Little-endian coco int */

#define MAX_RFILES	64
#define MAX_LFILES	64

struct ext_ref
{
	struct ext_ref *next;
	char            name[SYMLEN + 1];
};

struct exp_sym
{
	struct exp_sym *next;
	char            name[SYMLEN + 1];
	char            flag;
	unsigned        offset;
};

struct ob_files
{
	struct ob_files *next;
	char           *filename;
	FILE           *fp;
	long            object;
	long            locref;
	binhead         hd;
	struct exp_sym *symbols;
	struct ext_ref *exts;
	char            modname[MAXNAME + 1];
	unsigned        Code,
	                IDat,
	                UDat,
	                IDpD,
	                UDpD;
};

/* Define DEBUG to get way more verbose output */
/* #define DEBUG */

#ifdef DEBUG
#define DBGPNT(x)	printf x
#else
#define DBGPNT(x)
#endif

extern int      getname();
extern unsigned getwrd();
extern int      chk_dup();
extern int      rm_exref();
extern int      check_name();
extern int      ftext();

extern	int     (*XXX_header)();
extern	int     (*XXX_body_byte)();
extern	int     (*XXX_body)();
extern	int     (*XXX_tail)();

#define mc(c) ((c)&0xff)
#define DEF 1
#define REF 2
