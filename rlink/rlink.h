#include <rof.h>

typedef unsigned u16;	/* Little-endian coco int */

/* module.c */
int create_decb_module();
int create_os9_module();

#define MAX_RFILES	32
#define MAX_LFILES	32

struct ext_ref
{
	struct ext_ref *next;
	char name[SYMLEN+1];
};

struct exp_sym
{
	struct exp_sym *next;
	char name[SYMLEN+1];
	char flag;
	unsigned offset;
};

struct ob_files
{        
	struct ob_files	*next;
	char *filename;
	FILE *fp;
	long object;
	long locref;
	binhead hd;
	struct exp_sym *symbols;
	struct ext_ref *exts;
	char modname[MAXNAME+1];
	unsigned Code, IDat, UDat, IDpD, UDpD;
};

/* Define DEBUG to get way more verbose output */

#ifdef DEBUG
#define DBGPNT(x)	printf x
#else
#define DBGPNT(x)
#endif

extern int getname();
extern unsigned getwrd();
extern int chk_dup();
extern int rm_exref();
extern int check_name();
extern int ftext();

#define mc(c) ((c)&0xff)
#define DEF 1
#define REF 2
