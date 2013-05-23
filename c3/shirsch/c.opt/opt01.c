/* ******************************************************************** *
 * opt_01.c - First file in c.opt - contains main ()                    *
 *                                                                      *
 * $Id:: opt01.c 69 2008-09-28 03:49:45Z dlb                         $  *
 *                                                                      *
 * ******************************************************************** */

#include "copt.h"

static direct int DoStats,
                  InstrCount,
                  LineCount,
                  VsctTy,   /* Flag type of psect we are in (globally)
                             * 0 = not in vsect, 1 = non-dp, 2 = dp   */
                  SectEnded;

FILE *InPath,
     *OutPath;
static FILE *D0569;     /* Is this necessary? It's merely a copy of OutPath */

extern direct int InstrBufCnt;
extern int MaxInstrct;

static unsigned int L0382 (
#ifdef __STDC__
    int
#endif
);
static int percent (
#ifdef __STDC__
    unsigned int, unsigned int
#endif
);
static void readfile ();
static void do_sect ();

int
#ifdef __STDC__
main (int argc, char **argv)
#else
main (argc, argv)
    int argc;
    char **argv;
#endif
{
    int _digit;
    char *_arg;
    char *_inpth;
    char *_outpth;

    InPath = stdin;
    OutPath = stdout;

    _inpth = _outpth = 0;

    ++argv;

    while (--argc)              /* L0243 */
    {
        if (* (_arg = *argv) == '-')        /* L0198 */     /* else L0225 */
        {
            /*++ _arg;*/

            switch (*(++_arg))
            {
                case '\0':       /* L01b5 */
                    DoStats = 1;
                    break;
                case 'i':       /* L01bd */
                    MaxInstrct = 0;

                    if (*(++_arg) != '=')
                    {
                        --(_arg);
                    }

                    while ( (_digit = *(++_arg)) )
                    {
                        MaxInstrct = (MaxInstrct * 10) + (_digit - '0');
                    }

                    break;
                default:        /* L0200 */
                    errexit ("unknown option '%s'\n", --_arg);
                    break;
            }
        }
        else    /* Parameter is a source (or output) filename */
        {
            if ( ! _inpth )    /* first non-opt parameter is input file */
            {
                _inpth = _arg;
            }
            else
            {
                if ( ! _outpth)    /* second non-opt param is output file */
                {
                    _outpth = _arg;
                }
                else
                {
#ifdef COCO
                    errexit ("too many files");
#else
                    errexit ("too many files", 0);
#endif
                }
            }
        }

        ++argv;
    }

    /* If Input file was specified, reset input stream */

    if (_inpth)
    {
        if ( ! (InPath = freopen (_inpth, "r", stdin)))
        {
            errexit ("can't open %s", (void *)_inpth);
        }
    }

    /* Redirect output if specified */

    if (_outpth)
    {
        if ( ! (OutPath = fopen (_outpth, "w")))
        {
            errexit ("can't open %s", _outpth);
        }
    }

    D0569 = OutPath;
    inittlist ();
    initvars ();
    initwwlist ();
    readfile ();

    if (DoStats)      /* else L037e */
    {
        unsigned int _loc02;
        unsigned int _loc00;

        fprintf (stderr, "statistics:\n");

        fprintf (stderr, "\ttotal instructions : %d\n", InstrCount);
        fprintf ( stderr, "\tlong branches :  %5d, %5d, %3d%%\n",
                          LbrTot, LbrToBr, percent ( LbrToBr, LbrTot) );
        
        fprintf ( stderr, "\tremoved       :         %5d, %3d%%\n",
                          CmdRemovd, percent (CmdRemovd, InstrCount));

        _loc00 = (LbrToBr * 2);
        _loc00 += L0382 (CmdRemovd);

        _loc02 = L0382 (InstrCount);
        fprintf (stderr, "\ttotal bytes   :  %5d, %5d, %3d%%\n",
                        _loc02, _loc00,
                        percent (_loc00, _loc02 ));
    }

#ifndef COCO
    return 0;
#endif
}

/* ************************************************************************ *
 * L0382 () - Returns an approximate conversion of instructions to bytes    *
 * ************************************************************************ */

static unsigned int
#ifdef __STDC__
L0382 (int parm1)
#else
L0382 (parm1)
    int parm1;
#endif
{
    return ( (parm1 + parm1) + ((parm1 * 3) / 5) );
}

/* **************************************************************** *
 * percent () - Returns the percentage value between the first      *
 *              integer value and the second                        *
 * Passed : (1) The numerator for the percent value                 *
 *          (2) The denominator (field) for the divide              *
 * Returns: The percent value betweeen the two                      *
 * **************************************************************** */

static int
#ifdef __STDC__
percent (unsigned int numratr, unsigned int field)
#else
percent (numratr, field)
    unsigned int numratr;
    unsigned int field;
#endif
{
#ifdef COCO
    if (field)
#else
        /* Actually, this should be for any system, including COCO,
         * as any value for field < 50 will cause a divide by zero error
         */
    if (field > 50)
#endif
    {
        return (numratr / ((field + 50) / 100));
    }

    return 0;
}

static void
readfile ()
{
    /* 228 bytes of auto storage */

    char _inbuf[100];
    char _lblnam[12];
    char _opcod[11];
    char _oprnd[91];
    LBLENT *curntlbl;
    LBLENT *_newlbl;
    LBLENT *_lastlbl;
    char *_curline;
#ifdef COCO
    int var4;       /* unused - may remove after debugging */
#endif
    int _curntvsct; /* vsect flag for current line (see var VsctTy) */
    int _isglbl;
    register char *_curchr;

    curntlbl = 0;
    VsctTy = SectEnded = 0;
    LineCount = 0;

    /* read each line of the file */

    while (fgets (_inbuf, sizeof (_inbuf), InPath))      /* @ L0625 */
    {
        _curchr = _inbuf;        /* L03d7 */

        /* Chomp trailing newline */

        while (*_curchr != '\n')
        {
            /* The following probably won't ever happen unless an
             * error occurs, or a bad input file or stream.  The following
             * is an attempt avoid a segfault in case this happens.
             * As a matter of fact, this might be a good implementation
             * for the COCO if this source is ever used on it.
             */
#ifndef COCO
            if (_curchr == &(_inbuf[sizeof (_inbuf) - 1]))
            {
                break;
            }
#endif

            ++_curchr;
        }

        *_curchr = '\0';
        ++LineCount;
        _curline = _inbuf;

        if ((*_curline == '\0') || (*_curline == '*'))
        {
            continue;       /* Empty line or comment, ignore */
        }

        /* fetch label & opcode*/

        _curline = fetchfield ( 2, _opcod,      /* this fetch is for label */
                                (_curline = fetchfield ( 1,  /* for opcode */
                                                         _lblnam,
                                                         _curline,
                                                        &_isglbl) )
#ifndef COCO
                , 0     /* For profiler */
#endif
                );

        /* Move _curline past any blanks */
        while ((*_curline == ' ') || (*_curline == '\t'))
        {
            ++_curline;         /* L0432 */
        }

        /* handle directives */

        switch (*_opcod)        /* @ L0542 */
        {
            case 'e':           /* L0452 */
                if ( ! strcmp (_opcod, "endsect"))   /* else L056c */
                {
                    do_sect ();
                    SectEnded = 1;
                    continue;
                }

                break;
            case 'i':           /* L0473 */
                if ( ! strcmp (_opcod, "info"))
                {
                    goto prtinbuf;
                }

                break;
            case 'n':           /* L048c */
                if ( ! strcmp (_opcod, "nam"))
                {
prtinbuf:
                    fprintf (OutPath, "%s\n", _inbuf);
                    continue;
                }

                break;
            case 'p':           /* L04c2 */
                if ( ! strcmp (_opcod, "psect"))
                {
                    do_sect ();
                    goto prtinbuf;
                }

                break;
            case 't':           /* L04de */
                if ( ! strcmp (_opcod, "ttl"))
                {
                    goto prtinbuf;
                }

                break;
            case 'v':           /* L04f5 */
                if ( ! strcmp (_opcod, "vsect"))
                {
                    if ((*_curline == 'd') && (_curline[1] == 'p'))
                    {
                        _curntvsct = 2;
                    }
                    else
                    {
                        _curntvsct = 1;
                    }

                    if ((VsctTy == _curntvsct) && (SectEnded))
                    {
                        SectEnded = 0;
                        continue;
                    }

                    do_sect ();
                    VsctTy = _curntvsct;
                    goto prtinbuf;
                }

                break;
        }

        do_sect ();           /* L056c */

        if (VsctTy)     /* If we're still in a vsect, */
        {               /* simply print line and go process next */
            fprintf (D0569, "%s\n", _inbuf);
            continue;
        }

        if (_lblnam[0])                              /* else L05cb */
        {
            /* get this label name's LBLENT if already defined, or create one */

            if ( ! (_newlbl = findlbl (_lblnam)))        /* else L05a5 */
            {
                _newlbl = newlblref (_lblnam);
            }

            if (_isglbl)           /* L05a5 */
            {
                _newlbl->globlflg |= 1;
            }
        
            /* Find last label in the chain */

            _lastlbl = _newlbl;

            while (_lastlbl->nextme)
            {
                _lastlbl = _lastlbl->nextme;
            }

            _lastlbl->nextme = curntlbl;    /* add curntlbl to end of chain */
            curntlbl = _newlbl;             /* _newlbl is now current       */
        }

        if (*_opcod)    /* do operand, if any */      /* L05cb */
        {
#ifdef COCO
            fetchfield (3, _oprnd, _curline);
#else
            fetchfield (3, _oprnd, _curline, 0);
#endif
            
            if (bldcmdent (&D0015, _opcod, _oprnd, &curntlbl)) /* else L0612 */
            {
                doaltcode (D0015.prevcmd);
                optimizecode (D0015.prevcmd);
            }

            ++InstrCount;        /* L0612 */

            if (InstrBufCnt >= MaxInstrct)
            {
                prtlblcmd ();
            }
        }
    }       /* end while fgets */

    if (curntlbl)
    {
        bldcmdent (&D0015, "nop", "", &curntlbl);
    }

    do_sect ();       /* L0660 */

    while (InstrBufCnt)
    {
        prtlblcmd ();
    }
}

static void
do_sect ()
{
    if (SectEnded)
    {
        if (VsctTy)
        {
            VsctTy = 0;
        }
        else
        {
            while (InstrBufCnt)
            {
                prtlblcmd ();
            }
        }

        fprintf (OutPath, " endsect\n");
        SectEnded = 0;
    }
}
