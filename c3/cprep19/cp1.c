#include "cp.h"

/* The Following is sort of a Kludge for MinGW.
 * This requires that "/dd/defs" be installed in the Windows Root directory,
 * and you probably need to include a line "C:\dd    /dd" in your msys
 * /etc/fstab in order to access it from within msys
 */

#ifdef __MINGW32__
#  define INCLDIR "C:\\dd\\defs\\"
#else
#  define INCLDIR "/dd/defs/"
#endif
        /* open include files and place in file path stack */
void
doinclude (char *ln)
{
    int c;

    incln[fptr] = _line_;
    
    if (fptr == MAX_INCLUDE - 1)        /* Too many inc files -- FATAL ERROR */
    {
        doerr (7, ln - line);
        return;
    }
    
    if (*ln == '"')             /* File is in current working dir */
    {
        /* Get last char in file name + 1 */
        
        if ((c = findstr (2, ln, "\"") - 2) > 0)
        {
            strncpy (ifnbuf[++fptr], ln + 1, &ln[c] - ln);      /* Put filename in buf */
            ifnbuf[fptr][&ln[c] - ln] = 0;
            
            if ( ! (fpath[fptr] = fopen (ifnbuf[fptr], "r")))
            {
                doerr (6, ln - line);   /* Bad file */
                --fptr;     /* Ignore this file */
            }
        }
        else
            doerr (19, ln - line);      /* No end " */
    }
    else if (*ln == '<')        /* File is in DEFS dir */
    {
        int avail;

	avail = sizeof (ifnbuf[0]);

        /*strcpy (ifnbuf[++fptr], "/dd/defs/");*/ /* add LIB prefix to filename */
        strcpy (ifnbuf[++fptr], INCLDIR);
	avail -= (strlen (INCLDIR) + 1); /* srlen remaining less ending NULL */

	/* c = strlen (ln) up to, but not including, the ">" */
	
        if ((c = findstr (2, ln, ">") - 2) > 0)
        {
            char *fnam = &ifnbuf[fptr][strlen(ifnbuf[fptr])];
            /*int baselen = &ln[c] - ln;*/
            
            /* If filename len too long, write error message
             * This may be confusing, but at least it will cause
             * the user to look for the problem.  If a partial
             * filename were to be copied, it's _possible_ that
             * a filename of that name _might_ exist
             */
            
            if (c > avail)
            {
                fprintf (stderr, "Filename too long\n");
                doerr (6, ln - line);
                --fptr;     /* Ignore this file */
            }
            else
            {
                /*strncpy (&ifnbuf[fptr][9], &ln[1], &ln[c] - ln);*/
                strncat (ifnbuf[fptr], &ln[1], c);
                /*ifnbuf[fptr][&ln[c] - ln + 9] = 0;*/
                fnam[c] = '\0';
                /*fprintf(stderr,"ERR:buf=%s\n",ifnbuf[fptr]); */
                
                if ( ! (fpath[fptr] = fopen (ifnbuf[fptr], "r")))
                {
                    fprintf (stderr, "Filename: %s\n", ifnbuf[fptr]);
                    doerr (6, ln - line);   /* Bad file */
                    --fptr;     /* Ignore this file */
                }
            }
        }
        else
            doerr (9, ln - line);       /* No end > */
    }
    else                        /* Check for token-sequence */
    {
        splittok (ln, 0);       /* split end of line into tokens */
        expln (ln, NULL, NULL);
/*      fprintf(stderr,"ERR:ln=%s\n",ln);   */
        if (*ln == '\"' || *ln == '<')  /* If now in proper format... */
            doinclude (ln);     /* open include file */
        else
            doerr (6, ln - line);       /* Bad file name */
    }

    lnprint (1, NULL);          /* print #line for new file */
    return;
}

void
tstdupdef (void)
{
    register int i;

    for (i = 0; i < defcntr - 1; ++i)
    {
        if (!strcmp (defnam[i], defnam[defcntr - 1]))   /* if name matches */
        {
                /* if tok matches */
            if (!strcmp (deftok[i], deftok[defcntr - 1]))
            {
                    /* if arg matches */
                if ( ! (defarg[i] && defarg[defcntr - 1]) ||
                        ! (strcmp (defarg[i], defarg[defcntr - 1])))
                {
                    --defcntr;  /* decrement def array ptr */
                    dptr = defnam[defcntr];     /* reset $trng ptr to prev. def. */
                    return;
                }
            }
            --defcntr;          /* decrement def array ptr */
            dptr = defnam[defcntr];     /* reset $trng ptr to prev. def. */
            doerr (11, 8);
            return;
        }
    }
}                               /* No match found */

int
doundef (char *ln)
{
    int c, d, i, j;
    char *sptr, *eptr;
/* Unforch, Jims code didn't work because the 'ln' passed didn't have
	any spaces attached until the splittok(ln,0) had been done, at
	which point it had spaces on BOTH ends. The defnam array holds
	each name with an ending space only, therefore none of the strcmp		utilitys ever found a match. The undef was never done as no match
	was ever found. However, PLEASE give Jim credit for the well
	organized code thats easy to fix. Gene Heskett, CE@WDTV
*/
    splittok (ln, 0);           /* tokenize identifier */
    ln++;                       /* this the fix */
    if (strlen (ln) > 33)       /* Adjust ident. length if exceeds 31 + 2 spaces */
    {
        ln[32] = ' ';
        ln[33] = '\0';
    }
    for (i = 5; i < defcntr; ++i)
    {
        if (!strcmp (defnam[i], ln))    /* Undef name found */
        {
/*          fprintf(stderr,"%s, defnmbr %d is being undefined as %s\n",ln,i,defnam[i]); */
            if (i == defcntr - 1)       /* last name in table */
            {
                dptr = defnam[i];
                --defcntr;
            }
            else
            {
                sptr = defnam[i];       /* where to start removal */
                eptr = defnam[i + 1];   /* where to get next define name */
                d = eptr - sptr;        /* num of chars to remove */
                do              /* Adjust $tring table */
                {
                    *sptr++ = *eptr++;  /* copy remainder of table over it */
                }
                while (eptr != dptr);   /* till end of table + next char */
                dptr -= d;      /* and fix dptr back that far too */
                for (j = i + 1; j < defcntr; ++j)       /* Adjust def arrays */
                {
                    defnam[j - 1] = defnam[j] - d;      /* fix each pointer to equal */
                    deftok[j - 1] = deftok[j] - d;      /* next pointer -d */
                    if (defarg[j])      /* if it had a defined value, move it */
                        defarg[j - 1] = defarg[j] - d;  /* down the list too */
                    else
                        defarg[j - 1] = NULL;
                }
                --defcntr;      /* and remove it from the count */
                return (killine ());
            }
        }
    }
    return 0;
}

void
splittok (                      /* Check ln for tokens and inserts 1 space after each */
             char *ln, int b)
{
    int c, d;
    char tmp[LINEMAX + 3], *tptr, *lnptr;

    /* fprintf(stderr,"START:b=%d line=|%s|\n",b,ln);  */
    d = b;
    lnptr = ln + b;
    strcpy (tmp, ln);
    if (b == 0 && *tmp != ' ')
        *(lnptr++) = ' ';
    for (;;)
    {
        tptr = tmp + b;
        while (*tptr == ' ')
        {
            *(lnptr++) = ' ';
            ++tptr;
        }
        if (!(*tptr))
        {
            *lnptr = 0;
/*          fprintf(stderr,"line2=|%s|\n",ln);  */
            return;
        }
        if (*tptr == 'L' && (*(tptr + 1) == '"' || *(tptr + 1) == '\''))
            b = qa2 (tmp, *(tptr + 1), tptr - tmp + 1) + 1;     /* point to char after str */
        else if (IDNT_INIT (*tptr))     /* if identifier */
            b = getident (tmp, tptr - tmp) + 1; /* point to char after ident. */
        else if (*tptr == '"' || *tptr == '\'') /* chars or strings */
            b = qa2 (tmp, *tptr, tptr - tmp) + 1;       /* point to char after string */
        else if (isdigit (*tptr) || *tptr == '.')       /* test for number */
            b = toknum (tmp, tptr);     /* point to char after number */
        else if ((c = tokopr (tmp, tptr - tmp)) != ERROR)       /* Check for legal operators */
            b = c + 1;          /* point to char after operator */
        else
            b = tptr - tmp + 1; /* Misc. token */
        strncpy (lnptr, tptr, &tmp[b] - tptr);
        lnptr += (&tmp[b] - tptr);
        *lnptr = '\0';
/*      fprintf(stderr,"b=%d ln=|%s|\n",b,ln);  */
        if (tmp[b] != ' ')
            *(lnptr++) = ' ';
    }
}

/*
findchar(s,c)    Search for char in string, return position of char in
char *s,c;       string (1-...) else NULL if no match
*/
int
findchar (char *s, int c)
{
    char *curpos = strchr (s, c);

    return (curpos ? (curpos - s + 1) : 0);
}

/*
#asm
findchar:
 pshs u
 ldu 4,s    (*s)
 ldx #0     (b=0)
 ldb 7,s    (c)
FCWHL
 tst ,u     (while (*s))
 lbeq FCEND
 leax 1,x   (++b)
 cmpb ,u+   (if (c==*s++))
 lbne FCWHL
 tfr x,d
 puls u,pc
FCEND
 clra       (return NULL)
 clrb
 puls u,pc
#endasm
*/

/*
findstr(pos,string,pattern)
int pos;
char *string, *pattern;

return int position of 1st matching char (1-...)
else NULL if no match
else ERROR if pattern is NULL pointer
*/

int
findstr (int pos, char *string, char *pattern)
{
    char *substr;

    if (*pattern == '\0')
    {
        return 0;
    }
    
    substr = strstr (&(string[pos - 1]), pattern);

    return (substr ? (substr - string + 1) : 0);
}
/*#asm
findstr:
 pshs u,y
 ldd 6,s  (pos)
 addd 8,s (string)
 subd #1
 tfr d,y  (string+pos)
 ldu 10,s (pattern)
 tst ,u
 beq NMATCH
LP2
 lda ,y+
 beq NMATCH
 cmpa ,u
 bne LP2
 leax ,y
 leau 1,u
LP3
 tst ,u
 beq MATCH2
 lda ,y+
 beq NMATCH
 cmpa ,u+
 beq LP3
 ldu 10,s
 leay ,x
 bra LP2
MATCH2
 tfr x,d
 subd 8,s
 puls u,y,pc
NMATCH
 clra
 clrb
 puls u,y,pc
#endasm*/

int
tokopr (                        /* tests for operator or separator, returns 0 if invalid */
           char *ln,            /* else position of last char. of operator */
           int b)
{
    int c, d, e;

    c = ln[b];
    d = ln[b + 1];
    e = ln[b + 2];
/*  fprintf(stderr,"TOKOPR: b=%d c=%c d=%c e=%c\n",b,c,d,e);    */
    if (((c == '<' && d == '<') || (c == '>' && d == '>')) && e == '=')
        b += 2;
    else if (c == '.' && d == '.' && e == '.')
        b += 2;
    else if (d == '=' && findchar ("-+<>=!*/%&^|", c))  /* All ops ending = */
        ++b;
    else if (c == '-' && (d == '>' || d == '-'))
        ++b;
    else if ((c == '+' && d == '+') || (c == '<' && d == '<')
             || (c == '>' && d == '>'))
        ++b;
    else if ((c == '&' && d == '&') || (c == '|' && d == '|')
             || (c == '#' && d == '#'))
        ++b;
    else if (!findchar ("()[].!~+-*&/%<>^|?:=,{};#", c))
        return ERROR;
/*  fprintf(stderr,"TOKOPR: (final) b=%d\n",b); */
    return b;
}

int
toknum (char *ln, char *lnptr)
{
    if (*lnptr == '.')          /* screen out . as a struct operator */
        if (lnptr > ln && (IDNT_TYPE ((*(lnptr - 1)))))
            return (++lnptr - ln);

    for (;;)
    {
        do
        {
            ++lnptr;
        }
        while (isalnum (*lnptr) || *lnptr == '.');
        if ((*lnptr != '+' && *lnptr != '-')
            || (*(lnptr - 1) != 'E' && *(lnptr - 1) != 'e'))
            return (lnptr - ln);
    }
}

void
initstdefs (void)
{
    defnam[0] = "__LINE__ ";
    defnam[1] = "__FILE__ ";
    defnam[2] = "__DATE__ ";
    defnam[3] = "__TIME__ ";
    defnam[4] = "__LINE__ ";  /* blank out STDC slot unless ANSI selected */
    defnam[5] = "COCO ";      /* Add def to identify COCO - do we need _COCO? */

    defarg[0] = NULL;
    defarg[1] = NULL;
    defarg[2] = NULL;
    defarg[3] = NULL;
    defarg[4] = NULL;
    defarg[5] = NULL;

    deftok[0] = NULL;
    deftok[1] = NULL;
    deftok[2] = _date_;
    deftok[3] = _time_;
    deftok[4] = "1";          /* Indicates ANSI compatible */
    deftok[5] = NULL;

              /* defcntr was initialized to 5 reset it here */
    defcntr = 6;
}

void
cncatstr (char *ln)
{
    int b;

    while ((b = findstr (1, ln, "\" \"")))
        strcpy (&ln[b - 1], &ln[b + 2]);
}
