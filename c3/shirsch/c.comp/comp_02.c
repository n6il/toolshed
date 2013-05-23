/* ******************************************************************** *
 * comp_02.c - the second part of c.pass1 source                        *
 * $Id:: comp_02.c 73 2008-10-03 20:20:29Z dlb                        $ *
 * ******************************************************************** */

#include "ccomp.h"

static void prnt_filname ();

static void displerr (
#ifdef __STDC__
    char *, char *
#endif
);

static void showline (
#ifdef __STDC__
    register int _linpos, char *txt, int _line
#endif
);

static void e_putlin (
#ifdef __STDC__
    char *
#endif
);

/* The size of the data to save to/retrieve from G18Current */
#define PARTLBL(v) sizeof (LBLDEF) - (sizeof (v->fnam) + sizeof (v->fnext))


/* ******************************************************************** *
 * null_lbldef () - Saves everything from the LBLDEF in the parameter   *
 *            to G18Current (or new struct) except fname and fnext.     *
 *            parameter) - except for label name & fnext and nulls out  *
 *            This same portion in the srcdef LBLDEF.                   *
 * Seemingly, this is primarily to save the parameters of a previously  *
 *      declared label name and reuse its space for a duplicate name.   *
 * ******************************************************************** */

void
#ifdef __STDC__
null_lbldef (register LBLDEF *srcdef)
#else
null_lbldef (srcdef)
    register LBLDEF *srcdef;
#endif
{
    LBLDEF *_origdef;
    LBLDEF *_destdef;
#ifdef COCO
    int __count;    /* used in memcpy routine */
#endif

    /* if G18Current has been established, use it
     * and point G18Current to it's ->lblprev.
     * Else create a new struct
     */

    if ( (_destdef = G18Current) )
    {
        G18Current = _destdef->lblprev;
    }
    else
    {
        _destdef = addmem (PARTLBL (srcdef));
    }

    _origdef = srcdef;

#ifdef COCO
    mem_cp (_origdef, _destdef, PARTLBL (srcdef));
    __count = (PARTLBL(srcdef))/(sizeof (int));

    while (__count--)
    {
        /* orig code does clra clrb std ,u++ */
        /* this increments u, then stores d -2,u */
        *(((int *)srcdef)++) = 0;
    }
#else
    memcpy (_destdef, _origdef, PARTLBL(srcdef));
    memset (srcdef, 0, PARTLBL(srcdef));
#endif

    /* we get warning here, since ftop is defines as LBLDEF ** */
    _origdef->ftop = _destdef;
}

/* **************************************************************** *  
 * fill_g18 () - Move "g18" data from base of LBLDEF tree           *
 *          structure into LBLDEF in the parameter.                 *
 * Exit Conditions:                                                 *
 *      (1) *G18Current is base->lblprev                            *
 *      (2) G18Current now points to base of this tree              *
 * **************************************************************** */

void
#ifdef __STDC__
fill_g18 (register LBLDEF *dstdef)
#else
fill_g18 (dstdef)
    register LBLDEF *dstdef;
#endif
{
    LBLDEF *tree_base;

    /* we get warning here since ftop is defined LBLDEF ** */
    tree_base = dstdef->ftop;   /* Point to base of lbldef tree */

    /* Copy data from def at base to our LBLDEF */
    /* COCO _does_ have memcpy().  We could eliminate this #ifdef */

#ifdef COCO
    mem_cp (tree_base, dstdef, PARTLBL (dstdef));
#else
    memcpy (dstdef, tree_base, PARTLBL (dstdef));
#endif

    tree_base->lblprev = G18Current;
    G18Current = tree_base;
}

/* Not needed for other systems, as we have lib function memcpy ()
 * (COCO does, too, but it's not used in original code
 */

#ifdef COCO
mem_cp (_src, _dest, siz)
    register char *_src;
    char *_dest;
    int siz;
{
    while (siz--)
    {
        *(_dest++) = *(_src++);
    }
}
#endif

static void 
#ifdef __STDC__
prnt_filname (void)
#else
prnt_filname ()
#endif
{
    displerr ("%s : ", CurFilName);
}

void 
#ifdef __STDC__
err_quit (char *p1)
#else
err_quit (p1)
    char *p1;
#endif
{
    reprterr (p1);
    fflush (stderr);
    quitcc();
}

void 
#ifdef __STDC__
multdef (void)
#else
multdef ()
#endif
{
    reprterr ("multiple definition");
}

void
#ifdef __STDC__
reprterr (char *_str)
#else
reprterr (_str)
    char *_str;
#endif
{
    showline (D003f - (int)InpBuf, _str, InpLinNum);
}

void
#ifdef __STDC__
comperr (CMDREF *p1, char *_errmsg)
#else
comperr (p1, _errmsg)
    CMDREF *p1;
    char *_errmsg;
#endif
{
    char _str[50];

    strcpy (_str, "compiler error - ");
    strcat (_str, _errmsg);
    err_lin (p1, _str);
}

void
#ifdef __STDC__
err_lin (register CMDREF *p1, char *_errmsg)
#else
err_lin (p1, _errmsg)
    register CMDREF *p1;
    char *_errmsg;
#endif
{
    showline (((int)p1->_lpos - (int)InpBuf), _errmsg, p1->_cline);
}

static void
#ifdef __STDC__
showline (register int _linpos, char *txt, int _line)
#else
showline (_linpos, txt, _line)
    register int _linpos;
    char *txt;
    int _line;
#endif
{
    prnt_filname ();
    displerr ("line %d  ", (char *)_line);  /* to satisfy prototype */
    displerr ("****  %s  ****\n", txt);

    if (_line == FileLine)      /* else L04f1 */
    {
        e_putlin (InpBuf);
        goto L0518;
    }
    else
    {
        if ((FileLine - 1) == _line)    /* else _20 (L0528) */
        {
            e_putlin (PrevLine);
L0518:
            while (_linpos > 0)     /* Space over to position in line */
            {
                e_putc (' ');
                --_linpos;
            }

            e_putlin ("^");
        }
    }

    if ((++ErrCount) > 30)
    {
        fflush (stderr);
        e_putlin ("too many errors - ABORT");
        quitcc();
    }

}

static void
#ifdef __STDC__
displerr (char *pmpt, char *val)
#else
displerr (pmpt, val, p3)
    char *pmpt;
    char *val;
#endif
{

#ifndef COCO
    fprintf (stderr, pmpt, val);
#else
    fprintf (stderr, pmpt, val, p3);
#endif
}

static void
#ifdef __STDC__
e_putlin (char *str)
#else
e_putlin (str)
    char *str;
#endif
{
    fputs (str, stderr);
    e_putc ('\n');
}

void
#ifdef __STDC__
e_putc (char ch)
#else
e_putc (ch)
    char ch;
#endif
{
    putc (ch, stderr);
}

/* **************************************************************** *
 * CCREFtoLeft () - Appends CurntCREF onto bottom left-most cr_Left *
 *      with all its parameters.                                    *
 *      Wherever in the tree that there is a cr_Right in a CMDREF,  *
 *      the cr_Right is copied to the cr_Left of the "parent".  If  *
 *      there was a cr_Left at this point, is becomes the cr_Left   *
 *      of the just-copied cr_Right. IOW, the cr_Right is inserted  *
 *      into the left side, between the parent and the old cr_Left. *
 *                                                                  *
 *      Examples:                                                   *
 *              old                             new                 *
 *              p1                            -  p1 -               *
 *          /        \                      /         \             *
 *        p1L       p1R                    p1R  <-    p1R           *
 *       /             \                  /  \       /   \          *
 *     p1LL           p1RR              p1L   *    p1L  p1RR <-     *
 *                                     /                            *
 *                                   p1LL                           *
 *                                  /                               *
 *                                CCREF                             *
 *   NOTE (*): The right side here is unchanged from the cr_Right   *
 *   version.  Actually, in the above, the two p1R's are one and    *
 *   the same.                                                      *
 *   As another explanation, CCREFtoLeft recursively calls itself   *
 *   and walks itself to the bottom of the tree, leftward first,    *
 *   does the same for each right branch.  Note in the above, that  *
 *   if there was a p1RL (p1R->L), that the p1L would come under    *
 *   this (actually, it would be the be the cr_Left for the         *
 *   leftmost element of the right branch, similar to the way       *
 *   CCREF is inserted in the left branch.                          *
 * **************************************************************** */

void
#ifdef __STDC__
CCREFtoLeft (register CMDREF *myref)
#else
CCREFtoLeft (myref)
    register CMDREF *myref;
#endif
{
    if (myref)
    {
        /* if exists left, (myref->cr_Left)->cr_Left = CurntCREF
         * CurntCREF = myref->cr_Left
         */
        CCREFtoLeft (myref->cr_Left);


        /* if exists right, (myref->cr_Right)->cr_Left = myref->cr_Left
         * CurntCREF = myref->cr_Right
         */
        CCREFtoLeft (myref->cr_Right);

        /* myref->cr_Left = CurntCREF
         * CurntCREF = myref
         */
        mk_lftcmd (myref);
    }
}

/* ************************************************************ *
 * mk_lftcmd () - Makes the CMDREF stored in CurntCREF the      *
 *      cr_left for the CMDREF passed as a parameter,           *
 *      and stores the parameter CMDREF into CurntCREF          *
 * ************************************************************ */

void
#ifdef __STDC__
mk_lftcmd (register CMDREF *myref)
#else
mk_lftcmd (myref)
    register CMDREF *myref;
#endif
{
    if (myref)
    {
        myref->cr_Left = CurntCREF;
        CurntCREF = myref;
    }
}

void
#ifdef __STDC__
CmdrefCpy (CMDREF *src, CMDREF *dest)
#else
CmdrefCpy (src, dest)
    CMDREF *src;
    CMDREF *dest;
#endif
{
#ifdef COCO
    mem_cp (src, dest, sizeof (CMDREF));
#else
    memcpy (dest, src, sizeof (CMDREF));
#endif
}

/* ******************************************************** *
 * is_vardef () - Returns TRUE if the definition is a       *
 *              variable definition or a typedef            *
 * ******************************************************** */

int 
#ifdef __STDC__
is_vardef (void)
#else
is_vardef ()
#endif
{
    if (D003f == C_BUILTIN)    /* else L062d */
    {
        switch (LblVal)
        {
            case FT_INT:
            case FT_CHAR:
            case FT_UNSIGNED:
            case FT_SHORT:
            case FT_LONG:
            case FT_STRUCT:
            case FT_UNION:
            case FT_DOUBLE:
            case FT_FLOAT:
                return 1;
            default:
                return 0;
        }
    }
    else        /* L062d */
    {
        if (D003f == C_USRLBL)        /* else _67 (L06cd) */
        {
            if (((LBLDEF *)LblVal)->fnccode == FT_TYPEDEF)
            {
                return 1;
            }
        }
    }

    return 0;
}

/* ********************************************************** *
 * is_sc_specifier () - Returns true if a the word is a       *
 *      storage class specification                           *
 * ********************************************************** */

int 
#ifdef __STDC__
is_sc_specifier (void)
#else
is_sc_specifier()
#endif
{
    if (D003f == C_BUILTIN)
    {
        switch (LblVal)
        {
            case FT_EXTERN:
            case FT_AUTO:
            case FT_TYPEDEF:
            case FT_REGISTER:
            case FT_STATIC:
            case FT_DIRECT:
                return 1;
        }
    }

    return 0;
}

/* ******************************************************************** *
 * MSBrshft2 () - Returns a decremented depth flag.  It does this by    *
 *      shifting the value right 2 bits, while keeping the 4 LSB        *
 * Passed:  The original value (LBLDEF->gentyp ?? )                     *
 * Returns: The resulting value                                         *
 * ******************************************************************** */

int
#ifdef __STDC__
MSBrshft2 (int p1)
#else
MSBrshft2 (p1)
    int p1;
#endif
{
    return (((p1 >> 2) & 0xfff0) + (p1 & 0x0f));
}

/* **************************************************************** *
 * incptrdpth () - Increments the pointer depth flagging.           *
 *          Shift the original value less 4 LSB left 2 bits,        *
 *          and adding 0x10                                         *
 * Passed: the original value to shift                              *
 * Returns: The updated value                                       *
 * **************************************************************** */

int
#ifdef __STDC__
incptrdpth (int p1)
#else
incptrdpth (p1)
    int p1;
#endif
{
    return ((((p1 & 0xfff0) << 2) + 0x10) + (p1 & 0x0f));
}

/* **************************************************************** *
 * numeric_op () -                                                  *
 * Passed:  C_* vartyp                                              *
 * **************************************************************** */

int 
#ifdef __STDC__
numeric_op (int vrtyp)
#else
numeric_op (vrtyp)
    register int vrtyp;
#endif
{
    if ((vrtyp >= C_UMOD) && (vrtyp <= C_U_GT)) /* 76 to 99 */
    {
        return 1;
    }
        
    return 0;
}

struct brktdef *
#ifdef __STDC__
prevbrkt (register struct brktdef *p1)
#else
prevbrkt (p1)
    register struct brktdef *p1;
#endif
{
    if (p1)
    {
        return p1->brPrev;
    }
        
    return 0;
}

/* ************************************************************ *
 * lookfor () - see if the current character in D003f matches   *      
 *      the char passed as the parameter.  If so, get next char *
 * Returns: 0 if char matches                                   *
 *          1 if match fails and sends error msg                *
 * ************************************************************ */

int
#ifdef __STDC__
lookfor (int needed)
#else
lookfor (needed)
    int needed;
#endif
{
    if (D003f == needed)
    {
        nxt_word ();
        return 0;
    }
    else
    {
        register int _chr;

        _chr = 0;

        /* Scan _chcod01 for ascii character to report */

        while (_chr < 128)
        {
            if (_chcod01[_chr] == needed)
            {
                break;
            }

            ++_chr;
        }

        xexpcted[0] = _chr;
        reprterr (xexpcted);
    }

    return 1;
}

/* ******************************************************** *
 * cmma_rbrkt () - Keep reading "words" from input stream   *
 *      until either a comma or RBracket is found           *
 * ******************************************************** */

void 
#ifdef __STDC__
cmma_rbrkt (void)
#else
cmma_rbrkt ()
#endif
{
    while ((D003f != C_SEMICOLON) && (D003f != C_RBRACE) && (D003f != -1))
    {
        nxt_word();
    }
}
