/* ******************************************************************** *
 * opt02.c - file 2 for c.opt                                           *
 *                                                                      *
 * $Id:: opt02.c 52 2008-08-26 14:59:09Z dlb                         $  *
 * ******************************************************************** */

#include "copt.h"

/* ******************************************************************** *
 * fetchfield () Parses a string (srcstr) and moves the current field   *
 *      (if present) to destination string.                             *
 * Passed:  (1) type 1=lbl, 2=opcode, 3=operand  4=receiver             *
 * Returns: The original string updated to point to the begin of the    *
 *          next field.                                                 *
 *          isglbl = TRUE if it's a global label, FALSE in any other    *
 *          case                                                        *
 * ******************************************************************** */

char *
#ifdef __STDC__
fetchfield (int wrdtyp, char *deststr, char *srcstr, int *isglbl)
#else
fetchfield (wrdtyp, deststr, srcstr, isglbl)
    int wrdtyp;
    register char *deststr;
    char *srcstr;
    int *isglbl;
#endif
{
    int _wrdlen;

    switch (wrdtyp)      /* L08f5 = "break" */
    {
        case 1:     /* get label (if so) */         /* L07b8 */
            _wrdlen = 11;

            /* A label begins at the first character */

            if ((*srcstr == ' ') || (*srcstr == '\t'))
            {
                break;
            }

            /* move legal label chars to deststr */

            do
            {
                *(deststr++) = *(srcstr++);     /* L07cf */

                if ((--_wrdlen == 0))
                {                                           /* else L07fc */
                    break;
                }
            } while ((_chcodes[(int)*srcstr] & 0x0f));

            /* parse past any extra characters */

            while ( (_chcodes[(int)*srcstr] & 0x0f))
            {
                ++srcstr;
            }

            if ((*isglbl = (*srcstr == ':')))   /* L081b */   /* else break */
            {
                ++srcstr;
            }

            break;
        case 2:     /* opcode */         /* L082c */
            _wrdlen = 10;

            /* Parse past any white-spaces */

            while ((*srcstr == ' ') || (*srcstr == '\t')) /* @ L083a */
            {
                ++srcstr;
            }

            /* move up to _wrdlen chars to dest */

            while ((_wrdlen--) && (_chcodes[(int)*srcstr] & 0x0f))
            {
                *(deststr++) = *(srcstr++);
            }

            /* ignore any extra chars */

            while (_chcodes[(int)*srcstr] & 0x0f)    /* @ L087c */
            {
                ++srcstr;
            }

            break;
        case 3:         /* operand */         /* L0890 */
            _wrdlen = 90;

            /* skip any whitespaces */

            while ((*srcstr == ' ') || (*srcstr == '\t'))
            {
                ++srcstr;
            }

            /* get up to _wrdlen chars */

            while ( (_wrdlen-- != 0) && (*srcstr))      /* @ L08ba */
            {
                *(deststr++) = *(srcstr++);
            }

            break;
        default:        /* L08cd */
            errexit ("parse called with bad type : %d", (char *)wrdtyp);
            break;
    }

    *deststr = '\0';  /* place null terminator */

    return srcstr;
}
