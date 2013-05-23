/* ******************************************************************** *
 * opt_05.c - File five for c.opt - contains main ()                    *
 *                                                                      *
 * $Id:: opt05.c 56 2008-09-09 00:10:21Z dlb                         $  *
 *                                                                      *
 * ******************************************************************** */

/* Define "MAIN" here so data will be defined in this file */

#define MAIN
#define CMDFLGS

#include "copt.h"
#include <ctype.h>

/* WWList may not be a struct ww *, but we'll try it for now */

static struct ww *WWList[128];

extern char *P_brnchCC[];

static void invertCC (
#ifdef __STDC__
    CMDENT *
#endif
);

static int matchstrng (
#ifdef __STDC__
char *regptr, char *parm2
#endif
);

static int combincode (
#ifdef __STDC__
    CMDENT *, CMDENT *
#endif
);

static void addr_codcombine (
#ifdef __STDC__
    LBLENT *, CMDENT *
#endif
);

void
initwwlist ()
{
    struct ww **listptr;
    register struct ww *treeptr = D0146;

    while (treeptr->w0)
    {
        listptr = & (WWList[str_sum (treeptr->ocod01)]);

        /* If there was a previous entry, at this pt. in WWList, record it */
        
        treeptr->wwprior = *listptr;
        *listptr = treeptr;
        ++treeptr;
    }
}

/* ************************************************************************ *
 * doaltcode () - Attempts to substitute alternate codes.                   *
 * ************************************************************************ */

void
#ifdef __STDC__
doaltcode (CMDENT *frstcmd)
#else
doaltcode (frstcmd)
    register CMDENT *frstcmd;
#endif
{
    CMDENT *_cmdnext;
    WW *myWW;
#ifdef COCO
    int var6;       /* Not used */
#endif
    int _found;
#ifdef COCO
    int var2;       /* Not used */
    int var0;       /* Not used */
#endif

    frstcmd = frstcmd->prevcmd;

L1118:
    for (;;)
    {
        if ((frstcmd == &D0015) || ((_cmdnext = frstcmd->nxtcmd) == &D0015))
        {
            return;     /* We are at the base of the tree */
        }

        if (_cmdnext->cmlbl)              /* else L11c3 */
        {
            /* Abort if not a branch or if it's bsr */

            if ( ! (isbrnch(frstcmd)) || ((frstcmd->cmdflgs & 0x3f) == 33) )
            {                                   /* else L12f1 */
                return;  /* break */
            }

            if (((LBLENT *)(frstcmd->oprandptr))->lblcmd == _cmdnext)
            {                                               /* else L1181 */
                /* point all lblcmd's @ frstcmd's address to _cmdnext */
                comnlblcmd (frstcmd, _cmdnext);
                removcmd (frstcmd);
                frstcmd = _cmdnext->prevcmd; /* back up one cmd and repeat */
                ++CmdRemovd;
                goto L1118; /* begin test again with new frstcmd */
            }
            else
            {
                if ( ! (ispcrel (frstcmd)) || (frstcmd->cmlbl != 0))
                {
                    return; /* either not pc-rel or cmlbl exists */
                }

                if ( ((frstcmd = frstcmd->prevcmd) != &D0015)          &&
                        /* Must be a conditional branch */
                     ((frstcmd->cmdflgs & 0x3f) > 33)                  &&
                     (((LBLENT *)frstcmd->oprandptr)->lblcmd == _cmdnext)   )
                {
                    invertCC (frstcmd);
                }

                return;
            }
        }

        /* We come here if _cmdnext->cmlbl == 0 */

        /* A WWList entry is determined by the str_sum() of the chars
         * in the mnemonic name.  Some mnems may produce the same sum.
         * If so, we need to walk backward to get all entries */

        myWW = WWList[str_sum (frstcmd->cmdop)];       /* L11c3 */
        _found = 0;

        while (myWW)        /* @ L125a */
        {
            if ( ( matchstrng (frstcmd->cmdop, myWW->ocod01))      &&
                 ( matchstrng (frstcmd->oprandptr, myWW->oprnd01)) &&
                 ( matchstrng (_cmdnext->cmdop, myWW->ocod02))     &&
                    
                    ( matchstrng ( _cmdnext->oprandptr,
                                   myWW->oprnd02 == (char *)1     ?
                                       (char *)frstcmd->oprandptr :
                                       myWW->oprnd02)
                    )
               )       /* else L1254 */
            {
                _found = 1;
                break;
            }

            /* Walk back through all elements in this WWList entry */
            myWW = myWW->wwprior;
        }

        if ( ! _found)   /* else return */
        {
            return;
        }

        switch ((int)(myWW->altopcod))
        {
            case 1:
                break;
            case 2:         /* L126c */
                strcpy (frstcmd->cmdop, _cmdnext->cmdop);
                break;
            default:        /* L1273 */
                strcpy (frstcmd->cmdop, myWW->altopcod);
                break;
        }

        switch ((int)(myWW->altoprnd))       /* L1295 */
        {
            case 0:         /* L129b */
                *(char *)(frstcmd->oprandptr) = '\0';
                break;
            case 1:
                break;
            case 2:         /* L12a2 */
                strcpy (frstcmd->oprandptr, _cmdnext->oprandptr);
                break;
            default:        /* L12a9 */
                strcpy (frstcmd->oprandptr, myWW->altoprnd);
        }

        removcmd (_cmdnext);
        ++CmdRemovd;

        switch (myWW->w0)
        {
            case 1:
                frstcmd = frstcmd->prevcmd;
                break;  /* continue */
            case 2:
                return;
        }
    }       /* end "for (;;)" loop */       /* go to L111a */
}

static void
#ifdef __STDC__
invertCC (CMDENT *mycmd)
#else
invertCC (mycmd)
    register CMDENT *mycmd;
#endif
{
    /* strcpy (): reverse Condition Code of mycmd->cmdop and move all
     * cmdop's except PCREL from nxtcmd
     */
    strcpy ( mycmd->cmdop,
             P_brnchCC[ (mycmd->cmdflgs =
                        ( (mycmd->cmdflgs & 0x3f) ^ 1) |
                            (((CMDENT *)(mycmd->nxtcmd))->cmdflgs & 0xfec0) ) &
                        0x1f]);

    /* If mycmd is prevcmd for oprandptr, remove oprandptr from sumlist */
    lbl_del (mycmd, mycmd->oprandptr);
    setupbrnch (mycmd, mycmd->oprandptr = (mycmd->nxtcmd)->oprandptr);
    removcmd (mycmd->nxtcmd);
    ++CmdRemovd;
}

/* **************************************************************** *
 * matchstrng () - A multipurpose strcmp - type routine.            *
 *      If parm2 == 0, return TRUE, doing nothing                   *
 *      Whenever "<" is encountered in parm2, check that following  *
 *          number is within Direct Page bounds                     *
 *      Assures that strcmp regptr and parm2 is a match             *
 * Returns: TRUE if parm2 == 0, "<" spec is legal, and strcmp of    *
 *          both parameters matches, and all of regptr is parsed    *
 *          FALSE in any of the above fails                         *
 * **************************************************************** */

static int
#ifdef __STDC__
matchstrng (char *regptr, char *parm2)
#else
matchstrng (regptr, parm2)
    register char *regptr;
    char *parm2;
#endif
{
    if ( ! parm2)
    {
        return 1;
    }

    while (*parm2)
    {
        /* This portion checks if a direct specification is within
         * bounds ( < 255 )
         */

        if (*parm2 == '<')      /* L136e */
        {
            if ( ! isdigit (*regptr))
            {
                return 0;
            }

            {
                unsigned int _totl;

                _totl = 0;

                do          /* L138f */
                {
                    _totl = (_totl * 10) + *(regptr++) - '0';
                } while (isdigit(*regptr));

                /* check this */
                if (_totl > 255)
                {
                    return 0;
                }
            }

            ++parm2;
        }
        else
        {
            if (*(regptr++) != *(parm2++))
            {
                return 0;
            }
        }
    }       /* end "while (*parm2)" - L13e1 */

    if (*regptr == '\0')
    {
        return 1;
    }

    return 0;
}

void
#ifdef __STDC__
optimizecode (CMDENT *commncmd)
#else
optimizecode (commncmd)
    register CMDENT *commncmd;
#endif
{
    CMDENT *_brcmd;
    LBLENT *_dstlbl;
    CMDENT *_cmdnxt;
    CMDENT *_newnxt;
    CMDENT *_oldnxt;

    if (commncmd->prevcmd != &D0015)        /* else L15da (return)*/
    {
        _oldnxt = 0;
        _dstlbl = commncmd->cmlbl;

        while ( _dstlbl)
        {
            _cmdnxt = _dstlbl->brdstcmd;

            while (_cmdnxt)
            {
                _brcmd = _cmdnxt->prevcmd;

                if (   (_brcmd->cmlbl)      &&
                       (ispcrel (_brcmd))   &&
                     ! (_brcmd->cmdflgs & LBR2BR) )      /* else L1447 */
                {
                    /* _newnxt = NewDest or create new block
                     * NewDest = _newnxt => nxtcmd */
                    _newnxt = getNewDest ();
                    _newnxt->nxtcmd = _oldnxt;
                    _oldnxt = _newnxt;
                    _oldnxt->prevcmd = _brcmd;
                }

                _cmdnxt = _cmdnxt->nxtcmd;        /* L1447 */
            }

            _dstlbl = _dstlbl->nextme;
        }   /* end while (_cmdnxt) */       /* go to L14d1 */

        while (_oldnxt)        /* @ L14d1 */
        {
            comnlblcmd (_brcmd = _oldnxt->prevcmd, commncmd);       /* L145f */

            if ( ((_brcmd = _brcmd->prevcmd) != &D0015)  &&
                 (isbrnch (_brcmd))                   )   /* branch */
            {               /* else L14c0 */
                if (ispcrel (_brcmd))
                {
                    removcmd (_brcmd->nxtcmd);       /* go to L14be */
                }
                else
                {
                    /* > 33 = "conditional branch" */

                    if ( ((_brcmd->cmdflgs & 0x3f) > 33)             &&
                         ((((LBLENT *)_brcmd->oprandptr)->lblcmd) == 
                                        (_brcmd->nxtcmd)->nxtcmd)       )
                    {
                        invertCC (_brcmd);
                    }
                }
            }

            _newnxt = _oldnxt->nxtcmd;        /* L14c0 */

            /* _oldnxt->nxtcmd = NewDest,
             * NewDest = _oldnxt */
            to_NewDest (_oldnxt);
            _oldnxt = _newnxt;
        }           /* end while _oldnxt @ L14d1 */

        if ( ! (ispcrel(commncmd->prevcmd)))         /* else L152a */
        {
            _dstlbl = commncmd->cmlbl;

            while (_dstlbl)        /* @ L1519 */
            {
                _cmdnxt = _dstlbl->brdstcmd;

                while (_cmdnxt)        /* @ L150f */
                {
                    if (ispcrel (_brcmd = _cmdnxt->prevcmd))
                    {
                        addr_codcombine (_dstlbl->nextme, _brcmd);
                    }

                    _cmdnxt = _cmdnxt->nxtcmd;
                }

                _dstlbl = _dstlbl->nextme;
            }

            addr_codcombine (commncmd->cmlbl, commncmd);
        }

        if (ispcrel (commncmd))     /* else L15da */
        {
            if (isbrnch (commncmd)) /* branch */      /* else L1571 */
            {
                if ((_brcmd = ((LBLENT *)(commncmd->oprandptr))->lblcmd))
                {           /* else L1562 */
                    if ( ! combincode (commncmd->prevcmd, _brcmd->prevcmd))
                    {                               /* else L15da */
                        addr_codcombine (_brcmd->cmlbl, commncmd);
                    }
                }
                else
                {           /* L1562 */
                    addr_codcombine (commncmd->oprandptr, commncmd);
                }
            }
            else
            {           /* L1571 */
                _brcmd = commncmd->prevcmd;

                while (_brcmd != &D0015)       /* @ L15ce */
                {
                    if (   (ispcrel (_brcmd))                         &&
                            /* Not LBR2BR or branch */
                         ! ( (_brcmd->cmdflgs & 0xa0))                &&
                         ! ( strcmp (commncmd->cmdop, _brcmd->cmdop)) &&
                         ! ( strcmp ((char *)(commncmd->oprandptr),
                                     (char *)(_brcmd->oprandptr)))    &&
                         (combincode (_brcmd->prevcmd, commncmd->prevcmd))   )
                    {
                        return;
                    }

                    _brcmd = _brcmd->prevcmd;    /* L15ca */
                }
            }
        }
    }
}

/* ******************************************************************** *
 * combincode () - Compares two strings of code to see if they match.   *
 *                 If so, delete frstcmd string and set up lbranch from *
 *                 frstcmd string to secondcmd string                   *
 * Returns: TRUE if replacement made else FALSE                         *
 * ******************************************************************** */

static int
#ifdef __STDC__
combincode (CMDENT *firstcmd, CMDENT *secondcmd)
#else
combincode (firstcmd, secondcmd)
    register CMDENT *firstcmd;
    CMDENT *secondcmd;
#endif
{
    char _optlbl[12];
    CMDENT *base_first;
    CMDENT *base_second;
    LBLENT *_newdest;
    int count = 0;

                /* abort if both parms are one and the same */
    
    if ((base_first = firstcmd) == (base_second = secondcmd))
    {
        return 0;
    }

    /* L160d */
    /* back up into both prevcmd's until following fails (basically two
     * paths are identical - except frstcmd only must not be LBR2BR)
     */

    while ( (  firstcmd != &D0015 )      &&
            ( secondcmd != &D0015 )      &&
            (  firstcmd != base_second ) &&
            ( secondcmd != base_first )  &&
            /* Both are branches */
            ((firstcmd->cmdflgs & 0x3f) == ((secondcmd->cmdflgs) & 0x3f)) &&
            ( ! (firstcmd->cmdflgs & LBR2BR)) &&

            ( (isbrnch (firstcmd) ?  /* else L1684 */

                ( (((CMDENT *)firstcmd->oprandptr)->cmlbl) ?  /* else L1678 */
                    ( ((LBLENT *)(firstcmd->oprandptr))->lblcmd ==
                        ((LBLENT *)(secondcmd->oprandptr))->lblcmd )  :
                        (firstcmd->oprandptr == secondcmd->oprandptr)
                )            :       /* else not branch */
            ( ( ! strcmp (firstcmd->cmdop, secondcmd->cmdop)) &&  /* L1684 */
              ( ! strcmp (firstcmd->oprandptr, secondcmd->oprandptr)) )) )
          )
    {
        firstcmd = firstcmd->prevcmd;
        secondcmd = secondcmd->prevcmd;
        ++count;
    }       /* begins at L160d */


    /* now forward one cmd for frstcmd */

    firstcmd = firstcmd->nxtcmd;    /* L16ba */

    if (count)  /* if we've backed up any */    /* else L175b */
    {
        /* forward one for secondcmd */
        base_second = secondcmd = secondcmd->nxtcmd;
        
        /* if neither has a cmlbl, provide one with a "_$.." lbl */

        if ( ! (_newdest = firstcmd->cmlbl) && ! (_newdest = secondcmd->cmlbl) )
        {                                       /* else L171d */
            _newdest = newlblref (getoptlbl (_optlbl));
            _newdest->lblcmd = secondcmd;
            secondcmd->cmlbl = _newdest;
        }

        /* Remove all (frstcmd) cmds forward up to original */

        while ((count-- >= 0))       /* @ L171d */
        {
            /* set secondcmd as cmdlbl for all lbls at frstcmd's addr */
            comnlblcmd (firstcmd, secondcmd);      /* L16f9 */
            firstcmd = firstcmd->nxtcmd;
            removcmd (firstcmd->prevcmd);
            ++CmdRemovd;
            secondcmd = secondcmd->nxtcmd;
        }

        /* We don't retrieve the returned CMDENT here, as we're only
         * interested in modifying firstcmd
         */
        bldcmdent (firstcmd, "lbra", _newdest->lablnam, 0);

        doaltcode (firstcmd);   /* Substitute codes if possible */
    
        /* Combine code for all lbls at this address */
        addr_codcombine (base_second->cmlbl, base_second);
        return 1;
    }

    return 0;
}

/* ******************************************************************** *
 * addr_codcombine () - Combine code for all labels with address        *
 *          common to thislbl                                           *
 * ******************************************************************** */

static void
#ifdef __STDC__
addr_codcombine (LBLENT *thislbl, CMDENT *constcmd)
#else
addr_codcombine (thislbl, constcmd)
    LBLENT *thislbl;
    CMDENT *constcmd;
#endif
{
    register CMDENT *_myprvcmd;
    CMDENT *_cmdnxt;

    /* Process all thislbl's at this address */

    while (thislbl)       /* @ L17a1 */
    {
        _cmdnxt = thislbl->brdstcmd;

        while (_cmdnxt)        /* @ L1795 */
        {
            _myprvcmd = _cmdnxt->prevcmd;      /* L176c */

            if ( (ispcrel (_myprvcmd)) && ! (_myprvcmd->cmdflgs & LBR2BR) )
            {                   /* else L1793 */
                if (combincode (_myprvcmd->prevcmd, constcmd->prevcmd))
                {
                    break;
                }
            }

            _cmdnxt = _cmdnxt->nxtcmd;
        }

        thislbl = thislbl->nextme;
    }
}
