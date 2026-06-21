/*
 *     Include header for make
 *        This program has been placed in the Public Domain.
 */

#include <stdio.h>		/* FILE, used by the prototypes below */

/* Standard-library prototypes for the modern host build.  Kept ahead of the
   OS9 "#define void int" below so the system headers are seen untouched. */
#ifdef unix
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <time.h>
#endif

/* Systems currently handled by this code */
/* One of these must be specified */
/* #define OS9 */
/* #define eon */
/* #define unix */

/* define the label DEBUG to include additional debugging information */
/* #define DEBUG  */


#ifdef OS9
#define uchar          char
/* NB: no "#define void int" - c3 has a real void keyword (and now parses an
   ANSI "(void)" empty parameter list); textually mapping void->int would turn
   "(void)" into "(int)", i.e. a phantom int parameter. */
#define fputc          putc
#else
#define uchar          unsigned char
#endif

#define bool           uchar
#define time_t         long
#define TRUE           (1)
#define FALSE          (0)
#define max(a,b)       ((a)>(b)?(a):(b))

/* System-specific constants */
/* We define them here to minimize casing in the main code */
#ifdef unix
#define ERRNOTFND	ENOENT
#define DEFSHELL	"/bin/sh"
#endif
#ifdef eon
#define ERRNOTFND	ER_NOTF
#define DEFSHELL        ":bin/esh"
#endif
#ifdef OS9
#define ERRNOTFND	E_PNNF
#define DEFSHELL        "shell"
#endif

/* This is where it gets the default make rules.  If undefined, the defaults
   are compiled in (see makerules(void) in rules.c, which has unix/OS9/eon arms).
   We compile them in on both the host (unix) and the c3/OS-9 cross-build so a
   freshly-built make is self-contained - no external /dd/sys/make.default
   needed.  Leave the file-based default only for the legacy eon target. */
#ifdef eon
#define DEFN0          "/dd/sys/make.default"
#endif

/* Default Makefile names. */
#define DEFN1          "makefile"
/* Alternate makefile name. */
#ifdef unix
#define DEFN2          "Makefile"
#endif
#ifdef eon
#define DEFN2          "Makefile"
#endif
/* OS9 is case insensitive, so we don't need to define "Makefile" */

#define LZ             (1024)                  /*  Line size  */



/*
 *     A name.  This represents a file, either to be made, or existant
 */

struct name
{
       struct name *           n_next;       /* Next in the list of names */
       char *                  n_name;       /* Called */
       struct line *           n_line;       /* Dependencies */
       char *                  n_impl;       /* Implicit depend for < macro */
       time_t                  n_time;       /* Modify time of this name */
       uchar                   n_flag;       /* Info about the name */
};

#define N_MARK         0x01                    /* For cycle check */
#define N_DONE         0x02                    /* Name looked at */
#define N_TARG         0x04                    /* Name is a target */
#define N_PREC         0x08                    /* Target is precious */
#define N_DOUBLE       0x10                    /* Double colon target */
#define N_OBJECT       0x20                    /* Name is an object file */
#define N_ERROR        0x40                    /* This subtree had an error */

/*
 *     Definition of a target line.
 */
struct line
{
       struct line *           l_next;         /* Next line (for ::) */
       struct depend *         l_dep;          /* Dependents for this line */
       struct cmd *            l_cmd;          /* Commands for this line */
};


/*
 *     List of dependents for a line
 */
struct depend
{
       struct depend *         d_next;         /* Next dependent */
       struct name *           d_name;         /* Name of dependent */
};


/*
 *     Commands for a line
 */
struct cmd
{
       struct cmd *            c_next;         /* Next command line */
       char *                  c_cmd;          /* Command line */
};


/*
 *     Macro storage
 */
struct macro
{
       struct macro *          m_next;         /* Next variable */
       char *                  m_name;         /* Called ... */
       char *                  m_val;          /* Its value */
       uchar                   m_flag;         /* Infinite loop check */
};

#ifdef OS9
extern char *          object_dir; /* Special place to check for objects*/
extern char *          rel_dir; /* Special place to check for .r files */
#endif

extern char *          myname;
extern struct name     namehead;
extern struct macro *  macrohead;
extern struct name *   firstname;
extern bool            silent;
extern bool            ignore;
extern bool            rules;
extern bool            dotouch;
extern bool            quest;
extern bool            domake;
extern bool            debug;
extern bool            force;
extern char            str1[];
extern char            str2[];
extern int             lineno;
extern char *          currmake;

char *                 fgets();
char *                 index();
char *                 rindex();
/* char *                 malloc(); */
extern int             errno;

char *                 getmacro(char *);
struct macro *         setmacro(char *, char *);
void                   input(FILE *, char *);
#ifdef OS9			/* c3 has no varargs: error/fatal are K&R there */
void                   error();
void                   fatal();
#else
void                   error(char *, ...);
void                   fatal(char *, ...);
#endif
int                    make(struct name *, int);
struct name *          newname(char *);
struct depend *        newdep(struct name *, struct depend *);
struct cmd *           newcmd(char *, struct cmd *);
void                   newline(struct name *, struct depend *, struct cmd *, bool);
char *                 suffix(char *);
void                   touch(struct name *);
void                   makerules(void);
char *                 gettok(char **);
void                   precious(void);
bool                   getline1(char *, FILE *);
void                   expand(char *);
void                   prt(void);
void                   circh(void);
void                   modtime(struct name *);
bool                   dyndep(struct name *);
