/* ************************************************************ *
 * ccomp_11.c                                                   *
 * Deals with outputting data to the output stream              *
 *                                                              *
 * comes from p1_10.c                                           *
 *                                                              *
 * $Id:: comp_11.c 73 2008-10-03 20:20:29Z dlb                $ *
 * ************************************************************ */

#include "ccomp.h"
#include <errno.h>

#ifndef COCO
#   include <unistd.h>
#endif

static void lbl_rmb (
#ifdef __STDC__
    LBLDEF *, int, char
#endif
);

static void wrtstrings ();

void
#ifndef __STDC__
endprog ()
#else
endprog (void)
#endif
{
    wrtstrings ();
    prt_endsect ();

    if (ErrCount)       /* else L5dff */
    {
        prt_opcode ("fail source errors");
    }
}

void
#ifndef __STDC__
rmbnolbl (parm1, parm2, parm3)
    int parm1;
    int parm2;
    int parm3;
#else
rmbnolbl (int parm1, int parm2, int parm3)
#endif
{
    prt_vsect (parm3);
    prt_loclbl (parm1);
    fprintf (OutPath, " rmb %d\n", parm2);
    prt_endsect ();
}

void
#ifndef __STDC__
globldata (lbldf, datsize, isDP)
    LBLDEF *lbldf;
    int datsize;
    int isDP;
#else
globldata (LBLDEF *lbldf, int datsize, int isDP)
#endif
{
    prt_vsect (isDP);
    lbl_rmb (lbldf, datsize, ':');
    prt_endsect ();
}

void
#ifndef __STDC__
localdata (lbldf, datsize, isDP)
    LBLDEF *lbldf;
    int datsize;
    int isDP;
#else
localdata (LBLDEF *lbldf, int datsize, int isDP)
#endif
{
    prt_vsect (isDP);
    lbl_rmb (lbldf, datsize, ' ');
    prt_endsect ();
}

static void
#ifndef __STDC__
lbl_rmb (lbldf, rmbsize, colon)
    LBLDEF *lbldf;
    int rmbsize;
    char colon;
#else
lbl_rmb (LBLDEF *lbldf, int rmbsize, char colon)
#endif
{
#ifdef COCO
    fprintf (OutPath, "%.8s%c rmb %d\n", lbldf->fnam, colon, rmbsize);
#else
    fprintf (OutPath, "%.12s%c rmb %d\n", lbldf->fnam, colon, rmbsize);
#endif
}

void
#ifndef __STDC__
prtprofil (parm1, parm2)
    char *parm1;
    int parm2;
#else
prtprofil (char *parm1, int parm2)
#endif
{
    prt_loclbl (D005e = parm2);
#ifdef COCO
    fprintf (OutPath, " fcc \"%.8s\"\n fcb 0\n", parm1);
#else
    fprintf (OutPath, " fcc \"%.12s\"\n fcb 0\n", parm1);
#endif
}

void
#ifndef __STDC__
func_prolog (ttlnm, isglobl, lablnum)
    register char *ttlnm;
    int isglobl;
    int lablnum;
#else
func_prolog ( char *ttlnm,  /* function name for "ttl" printout */
              int isglobl,
              int lablnum ) /* label # (for profiler usage if applicable) */
#endif
{
#ifdef COCO
    fprintf (OutPath, " ttl %.8s\n", ttlnm);
#else
    fprintf (OutPath, " ttl %.12s\n", ttlnm);
#endif
    prt_label (ttlnm, isglobl);
    prt_opcode ("pshs u");

    if ( ! NoStkck)
    {
        fprintf (OutPath, " ldd #_%d\n lbsr _stkcheck\n",
                          (StkLblNum = ++LblNum));
    }

    if (DoProfil)       /* else L5dff */
    {
        prt_bgnfld ("leax ");
        prt_loclbl (lablnum);
        prnt_strng (",pcr\n pshs x\n leax ");
        prt_lblnam (ttlnm);
        prnt_strng (",pcr\n pshs x\n lbsr _prof\n leas 4,s\n");
    }
}

void
#ifndef __STDC__
prt_profend ()
#else
prt_profend (void)
#endif
{
    if (DoProfil)
    {
        fprintf (OutPath,
         " pshs d\n leax _%d,pcr\n pshs x\n lbsr _eprof\n leas 2,s\n puls d\n",
         D005e);
    }
}

void
#ifndef __STDC__
prtstkreq ()
#else
prtstkreq (void)
#endif
{
    if ( ! NoStkck)
    {
        fprintf (OutPath, "_%d equ %d\n\n", StkLblNum, (BlkStkReq - D0013 - 64));
    }
}

void
#ifndef __STDC__
prnt_fcb ()
#else
prnt_fcb (void)
#endif
{
    prt_bgnfld ("fcb ");
}

/* ************************************ *
 * prnt_fdb () = L5d32                  *
 * ************************************ */

void
#ifndef __STDC__
prnt_fdb ()
#else
prnt_fdb (void)
#endif
{
    prt_bgnfld ("fdb ");
}

void
#ifndef __STDC__
bgncomment ()
#else
bgncomment (void)
#endif
{
    prnt_strng ("* ");
}

void
#ifndef __STDC__
prt_vsect (isdp)
    int isdp;
#else
prt_vsect (int isdp)
#endif
{
//    prt_opcode ( isdp ? "vsect dp" : "vsect");
    prt_opcode ( "section bss");
}

void
#ifndef __STDC__
prt_endsect ()
#else
prt_endsect (void)
#endif
{
    prt_opcode ("endsect");
}

/* ************************************************************ *
 * wrtstrings () - writes out the strings to the output stream  *
 * ************************************************************ */

static void
#ifndef __STDC__
wrtstrings ()
#else
wrtstrings (void)
#endif
{
    register int ch;

    if (stmpFP)
    {
        rewind (stmpFP);

        /* Should "-1" be "0" for COCO ??? */
        while ((ch = getc (stmpFP)) != -1)
        {
            putc (ch, OutPath);
        }

        if (ferror (stmpFP))
        {
            err_quit ("dumpstrings");
        }

        fclose (stmpFP);
        
        /* non-coco systems use tmpfile () which automatically deletes
         * the tempfile on close
         */

#ifdef COCO
        unlink (CstrTmp);
#endif
#ifdef OSK
        unlink (CstrTmp);
#endif
    }
}

void
#ifndef __STDC__
quitcc ()
#else
quitcc (void)
#endif
{
    int var0;

    var0 = (errno ? errno : 1);

    if (stmpFP)
    {
        fclose (stmpFP);
        
        /* non-coco (or OSK) systems use tmpfile () which
         * automatically deletes the tempfile on close
         */

#ifdef COCO
        unlink (CstrTmp);
#endif
#ifdef _OSK
        unlink (CstrTmp);
#endif
    }

    _exit (var0);
}
