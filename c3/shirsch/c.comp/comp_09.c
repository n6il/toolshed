/* ******************************************************** *
 * ccomp_09.c - Functions dealing with reading input file   *
 * comes from p1_09.c                                       *
 * $Id:: comp_09.c 71 2008-10-01 21:12:03Z dlb            $ *
 * ******************************************************** */


#include "ccomp.h"

/*extern char _chcod01[];*/

static char prepln ();
static int a_toi (
#ifdef __STDC__
    char *
#endif
);

static char *gtnxtlin ();

/* ******************************************************************** *
 * initbuf0() - startup routine to initialize certain buffers           *
 * ******************************************************************** */

void
#ifdef __STDC__
initbuf0 (void)
#else
initbuf0 ()
#endif
{
    CurLinPtr = InpBuf;
    InpBuf[0] = '\0';
    CurChr = ' ';
}

/* ************************************************************* *
 * skipblank () a getnxtch() function that bypasses whitespaces  *
 * ************************************************************* */

void 
#ifdef __STDC__
skipblank (void)
#else
 skipblank ()
#endif
{
    while ((CurChr == ' ') || (CurChr == '\t'))
    {
        getnxtch ();
    }
}

/* ***************************************************************** *
 * getnxtch () - places next character into the variable CurChr.     *
 *    gets next applicable line if current line is exhausted..       *
 * ***************************************************************** */

void
#ifdef __STDC__
getnxtch (void)
#else
getnxtch ()
#endif
{
    if ((CurChr = *(CurLinPtr++)) == '\0')
    {
        CurChr = prepln ();
    }
}

/* ********************************************************** *
 * prepln() - Processes control lines  (those beginning with  *
 *           "#".                                             *
 * ********************************************************** */

static char
#ifdef __STDC__
prepln (void)
#else
prepln ()
#endif
{
    int v6;
    int _prepcod;
    int _linpos;
#ifdef COCO
    int v0; /* unused - may delete when finished debugging */
#endif

    if (D0058 == 0)
    {
        D0058 = 1;
        CurLinPtr = "";     /* go to L44cc */
        return ' ';
    }

    D0058 = 0;      /* L42f9 */
    strcpy (PrevLine, InpBuf);     /* Save current line */
    
    for (;;)    /* L430e */
    {
        if ( ! (CurLinPtr = gtnxtlin()))
        {
            return -1;
        }
        
        if (*CurLinPtr == '#')          /* else L44c5 */
        {
            _prepcod = CurLinPtr[1];

            if ( ! (gtnxtlin()))        /* else L4403 */
            {
                return -1;
            }
                
            switch (_prepcod)     /* at $63BD */
            {
                case '5':                   /* Line # */     /* L4336 */
                    FileLine = a_toi (InpBuf);
                    continue;                       /* L430e */
                case '6':   /* asm comment */     /* L4345 */
                    bgncomment ();
                case '2':           /* raw asm */     /* L4348 */
                    fprintf (OutPath, "%s\n", InpBuf);
                    continue;
                case '7':           /* Line ## "filename" */
                    /* '8' is not a standard COCO directive, but is from
                     *     OSK "cpp".  It appears to signify the filename
                     *     to return to from an #include.  COCO simply
                     *     uses '7', but we will use it in order to try
                     *     to use OSK's "cpp".
                     */

                case '8':
                    /* L4354 */
                    strcpy (CurFilName, InpBuf);
                     
                    if ( gtnxtlin())
                    {
                        continue;
                    }
                    else
                    {
                        return -1;
                    }
                    
                case 'P':   /* root startup*/     /* L4371 */
                    strcpy (D078a, InpBuf);

                    if ( ! gtnxtlin())
                    {
                        return -1;
                    }

/*                    fprintf (OutPath, " psect %s,0,0,%d,0,0\n",
                                        D078a, a_toi (InpBuf)) ;
*/
                    fprintf (OutPath, " section code\n");
                    fprintf (OutPath, " nam %s\n", D078a);
                    continue;
                case '0':     /* L43c3 */  /* '0' */
                case '1':     /* L43c3 */  /* '1' */
                    strcpy (D078a, InpBuf);

                    if ( ! gtnxtlin())
                    {
                        return -1;
                    }

                    v6 = a_toi (InpBuf);

                    if ( ! gtnxtlin())
                    {
                        return -1;
                    }

                    _linpos = a_toi (InpBuf);

                    if ( ! gtnxtlin())
                    {
                        return -1;
                    }

                    if (v6)
                    {
                        printf ("%s : line %d ", CurFilName, v6);
                    }
                    else
                    {
                        printf ("argument : ");
                    }

                    printf ("**** %s ****\n", InpBuf);

                    if (*D078a)
                    {
                        puts (D078a);

                        while ( _linpos--)
                        {
                            putc (' ', stdout);
                        }

                        puts ("^");
                    }

                    if (_prepcod == '1')
                    {
                        exit (1);
                    }

                    continue;
                default:
                    continue;
            }
        }
        else
        {
                /* L44c5 here */
            ++FileLine;
            return ' ';
        }
    }
}

static int
#ifdef __STDC__
a_toi (register char *cptr)
#else
a_toi (cptr)
    register char *cptr;
#endif
{
    int c_cod;
    int _sum;

    _sum = 0;

    while ((_chcod01[(c_cod = (*(cptr++)))]) == '\x6b')
    {
        _sum = ((_sum * 10) + (c_cod - '0'));
    }

    return _sum;
}

/* *********************************************************** *
 * gtnxtlin() - reads in the next line from and normalizes it. *
 *      Stores it in data area "InpBuf"                        *
 * Returns: ptr to InpBuf on anything but a true read error.   *
 *          NULL on true read error                            *
 * *********************************************************** */

static char *
#ifdef __STDC__
gtnxtlin (void)
#else
gtnxtlin ()
#endif
{
    int _curch;
    register char *_line = InpBuf;

    if ((_curch = getc (InPath)) == -1)     /* else L4579 */
    {
        if (ferror (InPath))
        {
            fputs ("INPUT FILE ERROR : TEMPORARY FILE\n", stderr);
            exit (1);
        }

        return 0;
    }

    while (_line != D00ac)
    {
        switch (_curch)
        {
            case '\n':
            case -1:
                *_line = '\0';
                return InpBuf;
            default:
                *(_line++) = _curch;
                _curch = getc (InPath);
        }
    }

    InpLinNum = ++FileLine;
    InpLinStr = InpBuf;
    err_quit ("input line too long");
#ifndef COCO
    return 0;   /* to keep gcc from griping */
#endif
}

