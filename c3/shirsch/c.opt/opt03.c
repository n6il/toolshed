/* ******************************************************************** *
 * opt_03.c - Part 3 of  c.opt - contains main ()                       *
 *                                                                      *
 * This file consists of mostly utility functions                       *
 *                                                                      *
 * $Id:: opt03.c 56 2008-09-09 00:10:21Z dlb                         $  *
 * ******************************************************************** */

#include "copt.h"

direct LBLENT *LblPulled;
int MyLbl = 0;

static struct t04 LblList[128];

static void *alloc_24 ();

/* ******************************************************************** *
 * inittlist () - run before readfile(). Initializes Array of t04       *
 *      pointers.  Fills each member with a pointer to self             *
 * ******************************************************************** */

void
inittlist ()
{
    struct t04 *this_t = LblList;
    register int t_ofset;

    t_ofset = 0;

    while (t_ofset < sizeof (LblList) / sizeof (LblList[0]))
    {
        /* Fill each element with ptr to self */
        this_t->t04_00 = this_t->finalsum = (LBLENT *)this_t;
        ++this_t;
        ++t_ofset;
    }
}

/* ******************************************************************** *
 * newlblref () - Returns a pointer to an s24 struct                    *
 * Passed:  label name                                                  *
 *          Stores parameter "name" in lablnam                          *
 *          sumnxt = appropriate t04 element of LblList                 *
 *          sumprev = finalsum                                          *
 * Returns: ptr to new (or reallocated) s24 struct                      *
 * ******************************************************************** */

LBLENT *
#ifdef __STDC__
newlblref (char *strng)
#else
newlblref (strng)
    char *strng;
#endif
{
    register struct t04 *_sumlist;
    LBLENT *_newlbl;

    _newlbl = alloc_24 ();     /* Get a 24-byte block of memory */
    _sumlist = & LblList[str_sum (strng)];

    strcpy (_newlbl->lablnam, strng);

    /* FIXME : I think it would be OK to cast _sumlist */
    _newlbl->sumnxt = (LBLENT *)_sumlist;     /* Flags "end of list" */
    
    /* Addr of prev entry (LblList self if 1st)  */
    _newlbl->sumprev = _sumlist->finalsum;

    /* Point previous->sumnxt to this new label */
    (_sumlist->finalsum)->sumnxt = _newlbl;
    _sumlist->finalsum = _newlbl;   /* replace old finalsum with new label */
    
    return _newlbl;
}

LBLENT *
#ifdef __STDC__
findlbl (char *strng)
#else
findlbl (strng)
    char *strng;
#endif
{
    LBLENT *mylbl;
    struct t04 *_listend;

    _listend = & LblList [str_sum (strng)];
    mylbl = _listend->t04_00;

    /* FIXME : I think it will be OK to cast _listend */
    while (mylbl != (LBLENT *)_listend)
    {
        if ( ! strcmp (strng, mylbl->lablnam))
        {
            return mylbl;    /* If name matches, success */
        }

        mylbl = mylbl->sumnxt;
    }

    return 0;       /* No match found, return FAIL */
}

/* **************************************************************** *
 * str_sum () - compute the sum of the characters in a string       *
 * Returns: The 7-bit sum of the character string                   *
 * **************************************************************** */

int
#ifdef __STDC__
str_sum (char *strng)
#else
str_sum (strng)
    register char *strng;
#endif
{
    int _sum;

    _sum = 0;

    while ( (*strng != '\0') )
    {
        _sum += *(strng++);
    }

    return (_sum & 0x7f);
}

/* ******************************************************************** *
 * alloc_24 () - Returns a memory block for a LBLENT                    *
 *      Returns LblPulled if allocated, else allocates a block from memory  *
 * ******************************************************************** */

static void *
alloc_24 ()
{
    register LBLENT *regptr;

    if (( regptr = LblPulled))
    {
        LblPulled = regptr->sumnxt;
        /* Do the following differently to keep gcc from complaining */
#ifdef COCO
        regptr->lblcmd = regptr->brdstcmd = regptr->nextme = regptr->globlflg = 0;
#else
        regptr->lblcmd = (CMDENT *)(regptr->nextme = 0);
        regptr->brdstcmd = (CMDENT *)(regptr->globlflg = 0);
#endif
    }
    else
    {
        regptr = add_mem (sizeof (LBLENT));
    }

    return regptr;
}

/* ******************************************************************** *
 * pull_lbl () - Aborts if lbl contains a lblcmd or brdstcmd.           *
 *          Pull regptr out of the sum-chain and points prev/next       *
 *          neighbors to each other.                                    *
 *          Pulled label's sumnxt = old LblPulled                       *
 *          LblPulled = pulled-label                                    *
 * ******************************************************************** */

void
#ifdef __STDC__
pull_lbl (LBLENT *thislbl)
#else
pull_lbl (thislbl)
    register LBLENT *thislbl;
#endif
{
    if ( ! (thislbl->lblcmd) && ! (thislbl->brdstcmd) )
    {
        (thislbl->sumprev)->sumnxt = thislbl->sumnxt;
        (thislbl->sumnxt)->sumprev = thislbl->sumprev;
        thislbl->sumnxt = LblPulled;
        LblPulled = thislbl;
    }
}

/* ************************************************************ *
 * getoptlbl () - Returns a new optimizer label (preceded by    *
 *          "_$")  *                                            *
 * Passed:  String address to store new labelname               *
 *          Increments MyLbl for next labelname                 *
 * Returns: Address (same as address passed as the parameter)   *
 * ************************************************************ */

char *
#ifdef __STDC__
getoptlbl (char *strngaddr)
#else
getoptlbl (strngaddr)
    char *strngaddr;
#endif
{
    sprintf (strngaddr, "_$%d", ++MyLbl);
#ifndef COCO
    return strngaddr;
#endif
}
