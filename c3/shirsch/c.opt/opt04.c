/* ******************************************************************** *
 * opt_04.c - File four for c.opt                                       *
 *                                                                      *
 * $Id:: opt04.c 56 2008-09-09 00:10:21Z dlb                         $  *
 *                                                                      *
 * ******************************************************************** */

#define CMDFLGS
#include "copt.h"

/* A recursive set of pointers (a stack) for strings */

struct strngstk {
    struct strngstk *arch;
};

static direct CMDENT *NewDest = 0;
static direct struct strngstk *ShortStrg = 0;
static direct struct strngstk *LongStrng = 0;

direct int InstrBufCnt;

int MaxInstrct = 300;

char *P_brnchCC[] =         /* D0027 */
{
    "ra", "sr", "eq", "ne", "lt", "ge",
    "le", "gt", "lo", "hs", "ls", "hi",
    "pl", "mi", "cc", "cs", "vc", "vs"
};

/* Shorthand for address of end of P_brnchCC[] */
#define PBR_END sizeof(P_brnchCC)/sizeof(P_brnchCC[0])

extern FILE *OutPath;

static CMDENT * getcmdref ();

static void lbr_2_br (
#ifdef __STDC__
    CMDENT *, int
#endif
);

static char *alloc_string (
#ifdef __STDC__
    int siz
#endif
);

static void psh_string (
#ifdef __STDC__
    char *parm1
#endif
);

/* **************************************************************** *
 * lkuptblstr () - Search an array of strings for a match           *
 * Passed:  (1) - needle - string to match                          *
 *          (2) - haystk - Array to search (haystk moves up list)   *
 *          (3) - end - ptr to end (+1) of list to search           *
 * Returns: ptr to locatation in "haystk" if match, otherwise, 0    *
 * **************************************************************** */

static char **
#ifdef __STDC__
lkuptblstr (char *needle, char **haystk, char **end)
#else
lkuptblstr (needle, haystk, end)
    char *needle;
    register char **haystk;
    char *end;
#endif
{
    while (haystk < end)
    {
        if ( ! strcmp (needle, *haystk))
        {
            return haystk;
        }

        ++haystk;
    }

    return 0;
}

void
initvars ()
{
    D0015.nxtcmd = D0015.prevcmd = (void *)(&D0015);
    InstrBufCnt = D001f = (int)(CmdDelNxt = 0);
}

/* ************************************************************************ *
 * bldcmdent ()                                                             *
 *                                                                          *
 * Branch:                                                                  *
 *      cmdflgs : (offset into branch tbl | 0x60) | 0x200 if conditional    *
 *      oprandptr : ptr to lblentry                                         *
 * Any:                                                                     *
 *      cmdflgs | 0x100 implies pc-related                                  *
 * ************************************************************************ */

CMDENT *
#ifdef __STDC__
bldcmdent (CMDENT *oldcmd, char *opcode, char *operand, LBLENT **curlbl)
#else
bldcmdent (oldcmd, opcode, operand, curlbl)
    CMDENT  *oldcmd;
      char  *opcode,
            *operand;
    LBLENT **curlbl;
#endif
{
    LBLENT  *_brLbl;
      char  *_cmdstr;
#ifdef COCO
      char  *_tmpstr;  /* only used to determine strlen */
#endif
      char **_brCC;
       int   _strlngth;

    register CMDENT *thiscref;

    thiscref = getcmdref ();  /* get existing CMDENT or create a new one */

    if ((thiscref->cmlbl = (curlbl ? *curlbl : 0)))    /* else L0ac3 */
    {
        _brLbl = *curlbl;

        while (_brLbl)      /* Point _brLbl to last label in chain */
        {
            _brLbl->lblcmd = thiscref;
            _brLbl = _brLbl->nextme;
        }

        *curlbl = 0;    /* Null out label ptr in calling function, as it
                         * is now recorded */
    }

    strcpy (thiscref->cmdop, opcode);         /* L0ac3 */

#ifdef COCO
    /* This is simply strlen, which could be used on the COCO, too
     * _tmpstr is not used elsewhere in this function */
    if ((*(_tmpstr = operand) != '\0'))
    {
        while (*(++_tmpstr) != '\0');
    }

    _strlngth = _tmpstr - operand;            /* L0aeb */
#else
    _strlngth = strlen (operand);
#endif
    thiscref->nxtcmd = oldcmd;
    thiscref->prevcmd = oldcmd->prevcmd;
    (oldcmd->prevcmd)->nxtcmd = thiscref;
    oldcmd->prevcmd = thiscref;
    ++InstrBufCnt;
    
    switch (*(_cmdstr = thiscref->cmdop))
    {
        case 'l':   /* "l"branch? */           /* L0b18 */
            if ( *(++_cmdstr) != 'b')
            {
                break;
            }           /* fall through to simple branch */
        case 'b':           /* L0b26 */
            if ( ! ( _brCC = lkuptblstr ( & _cmdstr[1],
                                            P_brnchCC,
                                          & P_brnchCC[PBR_END]
                                        ) 
                   )
               )
            {
                break;
            }

                    /* cmdflgs = offset into table | 0x60 */

            thiscref->cmdflgs |= ( (_brCC - P_brnchCC) | LBR );
            strcpy (thiscref->cmdop, *_brCC);    /* cmdop = CC mnem */
            fetchfield (1, operand, operand, &_strlngth);
            _brLbl = findlbl (operand);    /* get lbl struct or create new */

            if (P_brnchCC == _brCC)         /* If simple "bra" */
            {                                               /* else L0be8 */
                if (_brLbl && (_brLbl->lblcmd))             /* else L0bb8 */
                {
                    comnlblcmd (thiscref, _brLbl->lblcmd);
                }

                if ( ! ( thiscref->cmlbl)                        &&
                       (thiscref->prevcmd != (CMDENT *)(&D0015)) &&
                       (ispcrel (thiscref->prevcmd))                )
                {
                    removcmd (thiscref);
                    ++ CmdRemovd;
                    return 0;
                }
                else
                {
                    thiscref->cmdflgs |= PCREL;
                }
            }

            ++ LbrTot;            /* L0be8 */

            if ( ! _brLbl)
            {
                _brLbl = newlblref (operand);
            }

            thiscref->cmdflgs |= GOTLBLENT;
            setupbrnch (thiscref, thiscref->oprandptr = _brLbl);
            break;
        case 'p':           /* L0c15 */
            if ( ! strcmp (_cmdstr, "puls"))
            {
                if ( ! strcmp (&operand[_strlngth - 3], ",pc"))
                {
                    thiscref->cmdflgs |= PCREL;
                }
            }

            break;
        case 'r':           /* L0c38 */
            if ( ! strcmp (_cmdstr, "rts"))
            {
                thiscref->cmdflgs |= PCREL;
            }

            break;
    }

    if ( ! (isbrnch (thiscref)))
    {
        strcpy (thiscref->oprandptr = alloc_string (_strlngth), operand);
    }

    return thiscref;
}

/* **************************************************************** *
 * getcmdref () - either retrieves current cmd reference, or adds   *
 *          a new cmdref if none available                          *
 * **************************************************************** */

static CMDENT *
getcmdref ()
{
    register CMDENT *regptr;

    if ( ! (regptr = CmdDelNxt))     /* else L0cc4 */
    {
        if ( (++D001f) > (MaxInstrct + 3))
        {
#ifdef COCO
            errexit ("run out of instructions");
#else
            errexit ("run out of instructions", 0);
#endif
        }

        regptr = add_mem (sizeof (CMDENT));
    }
    else
    {
        CmdDelNxt = regptr->nxtcmd;
        regptr->cmdflgs = (int)(regptr->oprandptr = 0);
    }

    return regptr;
}

/* ************************************************************************ *
 * setupbrnch () - Retrieve CMDENT * from NewDest or create a new one if    *
 *                 no NewDest.                                              *
 *        CMDENT * of parameter 1 becomes the prevcmd for this CMDENT *     *
 *        brdstcmd of parameter 2 becomes the nxtcmd                        *
 *        This new struct becomes the brdstcmd for the operand parm2        *
 * ************************************************************************ */

void
#ifdef __STDC__
setupbrnch (CMDENT *brnchcmd, LBLENT *oprnd)
#else
setupbrnch (brnchcmd, oprnd)
    CMDENT *brnchcmd;
    LBLENT *oprnd;
#endif
{
    register CMDENT *regptr;

    /* If NewDest contains a value, Retrieve value from NewDest
     * and store this CMDENT's nxtcmd into NewDest
     * If NewDest is NULL, create new 2-word block */
    regptr = getNewDest ();
    regptr->prevcmd = brnchcmd;
    regptr->nxtcmd = oprnd->brdstcmd;
    oprnd->brdstcmd = regptr;
}

/* ************************************************************************ *
 * comnlblcmd () - Replaces or inserts a common command into all the        *
 *            cmdlbl's for all labels at a common address, beginning with   *
 *            frstcmd.                                                      *
 * Passed:  (1) - frstcmd   = beginning CMDENT to parse.                    *
 *          (2) - commoncmd = The CMDENT * to insert into all LBLENT's at   *
 *                  this address                                            *
 * ************************************************************************ */

void
#ifdef __STDC__
comnlblcmd (CMDENT *frstcmd, CMDENT *commoncmd)
#else
comnlblcmd (frstcmd, commoncmd)
    CMDENT *frstcmd;
    CMDENT *commoncmd;
#endif
{
    LBLENT *_lastlbl;
    register LBLENT *_curlbl;

    if ( (frstcmd != commoncmd) && (_curlbl = frstcmd->cmlbl) )
    {
        do
        {
            _lastlbl = _curlbl;  /* Keep track of last valid lbl */  /* L0cfd */
            _curlbl->lblcmd = commoncmd;
        } while ((_curlbl = _curlbl->nextme));

        _lastlbl->nextme = commoncmd->cmlbl;
        commoncmd->cmlbl = frstcmd->cmlbl;
        frstcmd->cmlbl = 0;
    }
}

void
prtlblcmd ()
{
    LBLENT *_lbllist;
    register CMDENT *_cmdptr;

    if ((_cmdptr = D0015.nxtcmd) == (CMDENT *)(&D0015))
    {
#ifdef COCO
        errexit ("removing too many instructions");
#else
        errexit ("removing too many instructions", 0);
#endif
    }

    if ((islngbrnch (_cmdptr)) == LBR)   /* L0d39 */
    {
        lbr_2_br (_cmdptr, 4);
    }

    if ((_lbllist = _cmdptr->cmlbl))        /* L0d50 */
    {                                       /* else L0dab */
        lbr_2_br (_cmdptr, 5);      /* jump t0 L0da9 */
    }

    /* List all labels for this address on successive lines */

    while (_lbllist)
    {
        fprintf (OutPath, "%s", _lbllist->lablnam);

        if (_lbllist->globlflg & 1)
        {
            putc (':', OutPath);
        }

        /* If another label in list, print CR */
        
        if ((_lbllist = _lbllist->nextme))
        {
            putc ('\n', OutPath);
        }
    }

    putc (' ', OutPath);    /* for final label, print cmd on that line */

    /* Handle (l)branches */

    if (isbrnch (_cmdptr))          /* else L0e0d */
    {
        fprintf ( OutPath, "%sb%s %s",
                  _cmdptr->cmdflgs & LBRANCH ? "l" : "",
                  _cmdptr->cmdop,
                  oprndislblent (_cmdptr) ?
                          ((LBLENT *)_cmdptr->oprandptr)->lablnam :
                          (char *)_cmdptr->oprandptr
                );
    }
    else
    {
        fprintf (OutPath, "%s", _cmdptr->cmdop);        /* L0e0d */

        if (((char *)_cmdptr->oprandptr)[0])
        {
            fprintf (OutPath, " %s", (char *)(_cmdptr->oprandptr));
        }
    }

    putc ('\n', OutPath);           /* L0e42 */
    removcmd (_cmdptr);
}

/* ******************************************************************** *
 * lbr_2_br () - Attempt to convert long branch to 8-bit branch         *
 * Passed:  (1) CMDENT *src_cmd = command from whence branch originates *
 *          (2) int flg - 4 = Search for a forward reference            *
 *                        5 = Search for a backward reference           *
 * If a branch - dest is within (an arbitrarily selected) 36 commands,  *
 * the long branch is reset to a (short) branch, and for forward br's   *
 * cmdflags for the destination is or'ed with 0x80                      *
 * ******************************************************************** */

static void
#ifdef __STDC__
lbr_2_br ( CMDENT *src_cmd, int flg)
#else
lbr_2_br (src_cmd, flg)
    CMDENT *src_cmd;
    int flg;
#endif
{
    CMDENT *_dst_cmd;
    LBLENT *_destlbl;
    int cmdcount;
    register CMDENT *_curntcmd = src_cmd;   /* Begin at src_cmd */

    if (flg == 4)       /* Search for a forward reference */
    {                   /* else L0ec9 */
        _destlbl = (oprndislblent (_curntcmd)) ?
                      (LBLENT *)_curntcmd->oprandptr :
                      findlbl (_curntcmd->oprandptr);

        /* if there _is_ a _destlbl and it has a lblcmd */

        if (_destlbl && (_dst_cmd = _destlbl->lblcmd))
        {           /* else return */
            cmdcount = 36;

            /* search through the next 36 commands
             * if the destination is within this range,
             * set cmdflgs for the dest | 0x80
             * unset "Long" branch for the originating branch
             */

            while ( (cmdcount != 0) &&
                    ((_curntcmd = _curntcmd->nxtcmd) != &D0015) )
            {
                if (_curntcmd == _dst_cmd)     /* else L0ead */
                {
                    _curntcmd->cmdflgs |= LBR2BR;
                    src_cmd->cmdflgs &= (-1 ^ LBRANCH);/* unset "long" branch */
                    ++LbrToBr;
                    return;     /* break */
                }

                --cmdcount;
            }
        }
    }
    else        /* Search for a backward reference */
    {
        cmdcount = 36;      /* L0ec9 */

        /* Search through the next 36 cmds for a backward branch
         * to the src cmd
         */

        while ((cmdcount != 0) && ((_curntcmd = _curntcmd->nxtcmd) != &D0015))
        {       /* @ L0f12 */
            if ( ((islngbrnch (_curntcmd)) == LBR)          &&
                 (_destlbl = (oprndislblent (_curntcmd)) ?
                        (LBLENT *)_curntcmd->oprandptr   :
                        findlbl (_curntcmd->oprandptr)    ) &&
                 (src_cmd == _destlbl->lblcmd)                 )
            {               /* else L0f0d */
                _curntcmd->cmdflgs &= (-1 ^ LBRANCH);  /* reset lbr to br */
                ++LbrToBr;
                return;     /* break */
            }

            --cmdcount;
        }
    }
}

void
#ifdef __STDC__
removcmd (CMDENT *delcmd)
#else
removcmd (delcmd)
    register CMDENT *delcmd;
#endif
{
    LBLENT *_lblptr;

    if (isbrnch (delcmd))
    {
        lbl_del (delcmd, delcmd->oprandptr);
    }
    else
    {
        psh_string (delcmd->oprandptr);
    }

    _lblptr = delcmd->cmlbl;

    while (_lblptr)        /* @ L0f65 */
    {
        _lblptr->lblcmd = 0;

        /* Remove _lblptr from sum-chain,
         * LblPulled => sumnxt
         * _lblptr => LblPulled */
        pull_lbl (_lblptr);
        _lblptr = _lblptr->nextme;
    }

    (delcmd->prevcmd)->nxtcmd = delcmd->nxtcmd;
    (delcmd->nxtcmd)->prevcmd = delcmd->prevcmd;
    delcmd->nxtcmd = CmdDelNxt;
    CmdDelNxt = delcmd;
    --InstrBufCnt;
}

/* ******************************************************************** *
 * lbl_del () - Checks if a command can be removed, and remove it if so *
 * Passed:  (1) - src_cmd : base cmd to compare                         *
 *          (2) - _dstlbl: LBLENT * found in an oprandptr, to trace     *
 * Beginning at _dstlbl->brdstcmd, trace through chain of cmds to see   *
 * if a cmd traces to src_cmd (1).  If so dstlbl is removed             *
 * ******************************************************************** */

void
#ifdef __STDC__
lbl_del (CMDENT *src_cmd, LBLENT *dstlbl)
#else
lbl_del (src_cmd, dstlbl)
    CMDENT *src_cmd;
    LBLENT *dstlbl;
#endif
{
    CMDENT *_newdest;
    CMDENT *_oldprev;

    if (dstlbl)      /* else return */
    {
        /* Init _oldprev so that _newdest can pick up "0,_oldprev"
         * on first iteration
         */

        _oldprev = (CMDENT *)&(dstlbl->brdstcmd);

        while ((_newdest = _oldprev->nxtcmd))
        {
            /* if src_cmd is same as prev cmd for _newdest */

            if (_newdest->prevcmd == src_cmd)
            {
                _oldprev->nxtcmd = _newdest->nxtcmd;
                pull_lbl (dstlbl);  /* Pull dstlbl out of label-sum-chain */

                /* _newdest ->nxtcmd = NewDest,
                 * NewDest = _newdest */
                to_NewDest (_newdest);
                break;  /* return */
            }

            _oldprev = _newdest;
        }
    }
}

/* ************************************************************************ *
 * getNewDest () - Returns value stored at NewDest if non-zero,             *
 *                      and repoints NewDest to its nxtcmd                  *
 *                 else creates a 2-int-sized memory block.                 *
 * ************************************************************************ */

CMDENT *
getNewDest ()
{
    register CMDENT *cmd;

    if ((cmd = NewDest))
    {
        NewDest = cmd->nxtcmd;
    }
    else
    {
        cmd = add_mem (sizeof (struct t04));
    }

    cmd->nxtcmd = cmd->prevcmd = 0;
    return cmd;
}

/* ************************************************************************ *
 * to_NewDest () - Stores value from NewDest to cmd->nxtcmd and saves       *
 *              cmd to NewDest                                              *
 * ************************************************************************ */

void
#ifdef __STDC__
to_NewDest (CMDENT *cmd)
#else
to_NewDest (cmd)
    register CMDENT *cmd;
#endif
{
    cmd->nxtcmd = NewDest;
    NewDest = cmd;
}

/* ******************************************************************** *
 * alloc_string () - If the appropriate string stack (either ShortStrng * 
 *      or LongStrng) has been established, pull the last entry off     *
 *      the stack.  If it hasn't been established, reserve the          *
 *      appropriate space and null the first entry.                     *
 * Passed : The length of the string                                    *
 * Returns: The address pulled of the string stack or the new space     *
 *          reserved                                                    *
 * ******************************************************************** */

static char *
#ifdef __STDC__
alloc_string (int siz)
#else
alloc_string (siz)
    int siz;
#endif
{
    /* FIXME : if we declare ShortStrg and LongStrng as char **,
     * we might eliminate the warnings.  Will leave it as is so
     * the warnings will point to this.
     */
    register struct strngstk *_newstrng;

    if (siz < 16)     /* else L1006 */
    {
        if ((_newstrng = ShortStrg))
        {
            ShortStrg = _newstrng->arch;
        }
        else
        {
            _newstrng = add_mem (16);         /* L1001 */
        }
    }
    else
    {
        if ((_newstrng = LongStrng))   /* else L1010 */
        {
            LongStrng = _newstrng->arch;
        }
        else
        {
            _newstrng = add_mem (91);
        }
    }

    _newstrng->arch = 0;
    return (char *)_newstrng;
}

/* ******************************************************************** *
 * psh_string () - If parameter is NULL, return doing nothing.          *
 *      Pushes the address of the string passed as a parameter onto     *
 *      the appropriate string-stack.  The string that is passed as     *
 *      the parameter is destroyed.                                     *
 * ******************************************************************** */

static void
#ifdef __STDC__
psh_string (char *strng)
#else
psh_string (strng)
    char *strng;
#endif
{
#ifndef COCO
    if (strng)
    {
        if (strlen (strng) < 16)
        {
            ((struct strngstk *)strng)->arch = ShortStrg;
            ShortStrg = (struct strngstk *)strng;
        }
        else
        {
            ((struct strngstk *)strng)->arch = LongStrng;
            LongStrng = (struct strngstk *)strng;
        }
    }
#else
    register char *tststr;

    if ((tststr = strng))       /* else return */
    {
        while ( *tststr)        /* strlen (), really */
        {
            ++tststr;
        }

        if ((tststr - strng) < 16)      /* else L1047 */
        {
            ((struct strngstk *)strng)->arch = ShortStrg; /* FIXME */
            ShortStrg = (struct strngstk *)strng;
        }
        else
        {
            ((struct strngstk *)strng)->arch = LongStrng; /* FIXME */
            LongStrng = (struct strngstk *)strng;
        }
    }
#endif
}
