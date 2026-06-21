/*
 *     Control of the implicit suffix rules
 *       This program has been placed in the Public Domain.
 */


#include <stdio.h>
#include "h.h"


/*
 *     Return a pointer to the suffix of a name
 */
char *
suffix(char *name)
{
       return rindex(name, '.');
}


/*
 *     Dynamic dependency.  This routine applies the suffis rules
 *     to try and find a source and a set of rules for a missing
 *     target.  If found, np is made into a target with the implicit
 *     source name, and rules.  Returns TRUE if np was made into
 *     a target.
 */
bool
dyndep(struct name *np)
{
       register char *         p;
       register char *         q;
       register char *         suff;           /*  Old suffix  */
       register char *         basename;       /*  Name without suffix  */
       struct name *           op;             /*  New dependent  */
       struct name *           sp;             /*  Suffix  */
       struct line *           lp;
       struct depend *         dp;
       char *                  newsuff;

       if (debug)
          printf("Trying to find implicit dependency for \"%s\".\n",np->n_name);

       if ((suff = suffix(np->n_name))==(char *)0)
       {
               strcpy(str1,np->n_name);  /* No suffix, copy full name */
               suff = "\0";     /* Set suffix to null string */
       }
       else
       {
               p = str1; q = np->n_name;
               while (q < suff)     /* Copy basename only */
                       *p++ = *q++;
               *p = '\0';
       }

       basename = setmacro("*", str1)->m_val;

       if (!((sp = newname(".SUFFIXES"))->n_flag & N_TARG))
               return FALSE;

       for (lp = sp->n_line; lp; lp = lp->l_next)
               for (dp = lp->l_dep; dp; dp = dp->d_next)
               {
                       newsuff = dp->d_name->n_name;
                       if (strlen(suff)+strlen(newsuff)+1 >= LZ)
                               fatal("Suffix rule too long");
                       strcpy(str1,newsuff);
                       strcat(str1,suff);
                       sp = newname(str1);
                       if (sp->n_flag & N_TARG)
                       {
                               if (strlen(basename) + strlen(newsuff)+1 >= LZ)
                                       fatal("Implicit name too long");
                               strcpy(str1,basename);
                               strcat(str1,newsuff);
#ifdef DEBUG
                               if (debug)
     printf("Checking \"%s\" as a possible implicit for \"%s\".\n"
                ,str1,np->n_name);
#endif
                               op = newname(str1);
                               if (!op->n_time)
                                       modtime(op);
                               if (op->n_time || (op->n_flag & N_TARG))
                               {
                                    if (debug)
                                printf("Found implicit \"%s\" for \"%s\".\n"
                                                ,op->n_name,np->n_name);
                                    dp = newdep(op, 0);
                                    newline(np, dp, sp->n_line->l_cmd, 0);
                                    np->n_impl = op->n_name;
                                    return TRUE;
                               }
                       }
               }
       return FALSE;
}

/* If there's no filename for a file with default rules, we compile them in */
#ifndef  DEFN0

/*
 *     Make the default rules
 */
void
makerules()
{
       struct cmd *            cp;
       struct name *           np;
       struct depend *         dp;


#ifdef eon
       setmacro("BDSCC", "asm");
       /*      setmacro("BDSCFLAGS", "");      */
       cp = newcmd("$(BDSCC) $(BDSCFLAGS) -n $<", 0);
       np = newname(".c.o");
       newline(np, 0, cp, 0);

       setmacro("CC", "c");
       setmacro("CFLAGS", "-O");
       cp = newcmd("$(CC) $(CFLAGS) -c $<", 0);
       np = newname(".c.obj");
       newline(np, 0, cp, 0);

       setmacro("M80", "asm -n");
       /*      setmacro("M80FLAGS", "");       */
       cp = newcmd("$(M80) $(M80FLAGS) $<", 0);
       np = newname(".mac.o");
       newline(np, 0, cp, 0);

       setmacro("AS", "zas");
       /*      setmacro("ASFLAGS", "");        */
       cp = newcmd("$(ZAS) $(ASFLAGS) -o $@ $<", 0);
       np = newname(".as.obj");
       newline(np, 0, cp, 0);

       np = newname(".as");
       dp = newdep(np, 0);
       np = newname(".obj");
       dp = newdep(np, dp);
       np = newname(".c");
       dp = newdep(np, dp);
       np = newname(".o");
       dp = newdep(np, dp);
       np = newname(".mac");
       dp = newdep(np, dp);
       np = newname(".SUFFIXES");
       newline(np, dp, 0, 0);
#endif

/*
 *     Some of the UNIX implicit rules
 */
#ifdef unix
       setmacro("CC", "cc");
       setmacro("CFLAGS", "-O");
       cp = newcmd("$(CC) $(CFLAGS) -c $<", 0);
       np = newname(".c.o");
       newline(np, 0, cp, 0);

       setmacro("AS", "as");
       cp = newcmd("$(AS) -o $@ $<", 0);
       np = newname(".s.o");
       newline(np, 0, cp, 0);

       setmacro("YACC", "yacc");
       /*      setmacro("YFLAGS", ""); */
       cp = newcmd("$(YACC) $(YFLAGS) $<", 0);
       cp = newcmd("mv y.tab.c $@", cp);
       np = newname(".y.c");
       newline(np, 0, cp, 0);

       cp = newcmd("$(YACC) $(YFLAGS) $<", 0);
       cp = newcmd("$(CC) $(CFLAGS) -c y.tab.c", cp);
       cp = newcmd("rm y.tab.c", cp);
       cp = newcmd("mv y.tab.o $@", cp);
       np = newname(".y.o");
       newline(np, 0, cp, 0);

       np = newname(".s");
       dp = newdep(np, 0);
       np = newname(".o");
       dp = newdep(np, dp);
       np = newname(".c");
       dp = newdep(np, dp);
       np = newname(".y");
       dp = newdep(np, dp);
       np = newname(".SUFFIXES");
       newline(np, dp, 0, 0);
#endif
#ifdef OS9
/*
 *     Default rules for the c3 toolchain.  The c3 driver runs the whole
 *     pipeline (c3prep | c3comp1 | c3comp2 | c3optx | rma | rlink) and writes
 *     an executable module named after the source.  -2 selects the two-pass
 *     compiler, the only one that fits an OS-9 Level-2 task.
 */
       setmacro("CC", "c3");
       setmacro("CFLAGS", "-2");

/*
 *     .a (rma assembler source) -> .r relocatable, via the standalone rma.
 */
       setmacro("AS", "rma");
       cp = newcmd("$(AS) $< -o=$@", 0);
       np = newname(".a.r");
       newline(np, 0, cp, 0);

/*
 *     .c -> executable module (no suffix): the c3 driver compiles and links.
 */
       cp = newcmd("$(CC) $(CFLAGS) $<", 0);
       np = newname(".c");
       newline(np, 0, cp, 0);

/*
 *     .SUFFIXES list
 */
       np = newname(".r");
       dp = newdep(np, 0);
       np = newname(".a");
       dp = newdep(np, dp);
       np = newname(".c");
       dp = newdep(np, dp);
       np = newname(".SUFFIXES");
       newline(np, dp, 0, 0);
#endif
}
#endif
