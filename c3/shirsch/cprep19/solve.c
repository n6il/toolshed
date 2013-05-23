/*
 psect solve_c,0,0,0,0,0
 nam solve_c
 ttl solve
*/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "proto.h"

int
solve (char *ln)
{
    int t_840;
    int t_838;
    int t_836;
    int t_834;
    int slv_stk[258];
    int t_316;
    char t_312[4];
    char t_272[40];
    char t_262[10];
    char **linptr;
    char filler[258];
    char *parmcpy;

    t_840 = t_838 = t_836 = slv_stk[0] = t_312[0] = 0;
    parmcpy = ln - 1;
    linptr = &parmcpy;
    
    while (parmcpy[1] != '\0')
    {
        t_834 = getop1 (filler, linptr);

        if (t_834 == 2)
        {
            pshunop (filler, t_262, &t_836);
            continue;
        }
        t_834 = getnum (&t_316, linptr);

        if (t_834 == 1)
        {
            pushstk (t_316, slv_stk, &t_840);

            while (t_836 != 0)
            {
                pushstk (calc1 (popstk (slv_stk, &t_840),
                                popunop (t_262, &t_836)),
                                slv_stk, &t_840);
            }

            while (*(t_312) != '\0')
            {
                if (tstnxtop (t_312, linptr))
                {
                     pushop (t_312, t_272, &t_838);
                     *t_312 = '\0';
                }

                else
                {
                    pushstk (calc2 (popstk (slv_stk, &t_840),
                            popstk (slv_stk, &t_840), t_312),
                                            slv_stk, &t_840);
                     
                    t_312[0] = '\0';

                    if (t_838 != 0)
                    {
                        strcpy (t_312, popop (t_272, &t_838));
                    }
                }
            }   /* end "while (*(t_312) != '\0')" */
        }

        if (!(t_834 = getopr (filler, linptr)))
        {
            break;
        }

        strcpy (t_312, filler);
    }       /* end "while (parmcpy[1] != '\0')" */

    if (t_840 == 1)
    {
        return (*slv_stk);
    }
    else
    {
        exit (_errmsg (0,"Stack val=%d.  equation error.\n" , t_840));
    }

    return 0;
}

void

pushstk (int p1, int *p2, int *p3)
{
    int stk;

    if (*p3 < 258)
    {
        p2[(*p3)++] = p1;
    }
    else
    {
        exit (_errmsg (0, "PUSHSTK: Stack full!\n"));
    }
}

int
popstk (int *p1, int *ptr)
{
    return (p1[--(*ptr)]);
}

void
pushop (char *p1, char *p2, int *p3)
{
    if (*p3 < 10)
    {
        strcpy (&p2[(*p3)++ * 4], p1);
    }
    else
    {
        exit (_errmsg (0, "PUSHOP: Stack full!\n"));
    }
}

char *
popop (char *p1, int *p2)
{
    if (*p2 > 0)
    {
        return (&p1[--(*p2) * 4]); 
    }
    else
    {
        exit (_errmsg (0, "Stack empty!\n"));
    }
}

void
pshunop (char *li, char *p2, int *p3)
{

    if (*p3 < 10)
    {
        p2[(*p3)++] = *li;
    }
    else
    {
        exit (_errmsg(0, "PSHUNOP: Stack full!\n"));
    }
}

char
popunop (char *p1, int *p2)
{
    if (*p2 > 0)
    {
        return (p1[--(*p2)]);
    }
    else
    {
        exit (_errmsg (0, "POPUNOP: Stack empty!\n"));
    }
}

int
getnum (int *parm1, char **ptr)
{
    int s_260;
    int s_258;
    char s_0[258];

    while (*(++(*ptr)) == ' ');
    
    if (**ptr == '(')
    {
        s_258 = 1;
        s_260 = 0;
        
        do
        {
            s_0[s_260++] = *(++(*ptr));
            
            if (**ptr == '(')
            {
                ++s_258;
            }
            else
            {
                if (**ptr == ')')
                {
                    --s_258;
                }
            }
        } while (s_258 != 0);
        
        s_0[s_260 - 1] = '\0';
        
        *parm1 = solve (s_0);
        return 1;
    }

    if (isdigit (**ptr))
    {
        *s_0 = **ptr;
        s_260 = 0;

        while (isdigit (s_0[++s_260] = *(++(*ptr))));
        
        s_0[s_260] = '\0';
        --(*ptr);
        *parm1 = atoi (s_0);
        return (1);
    }
    exit (_errmsg (0, "%d  Illegal number.\n", **ptr));
}

/* ************************************************* *
 * getopr() -                                        *
 * fixed                                             *
 * ************************************************* */

int
getopr (char *p1, char **p2)
{
    int myvar;
    
    while (*(++(*p2)) == ' ');
    
    myvar = 1;
    
    switch (**p2)
    {
        case '\0':
            return (0);
            break;
        case '|':
        case '&':
            *p1 = **p2;
            
            if ((*p2)[1] == (*p2)[0])
            {
                p1[myvar++] = **p2;
                ++(*p2);
            }

            p1[myvar] = 0;
            return 3;
            break;
        case '^':
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
            *p1 = **p2;
            p1[1] = 0;
            return 3;
            break;
        case '=':
        case '!':
            if ((*p2)[1] == '=')
            {
                *p1 = **p2;
                p1[1] = '=';
                p1[2] = '\0';
                ++(*p2);
                return 3;
            }
            exit (_errmsg (0, "%d Illegal binary operator.\n", (*p2)[1]));

            break;
        case '<':
        case '>':
            *p1 = **p2;

            if ((*p2)[1] == '=')
            {
                p1[myvar++] = '=';
                ++(*p2);
            }
            else
            {
                if ((*p2)[1] == (*p1))
                {
                    p1[myvar++] = *p1;
                    ++(*p2);
                }
            }
            
            p1[myvar] = 0;
            return 3;
            break;
        default:
                exit (_errmsg (0, "%d Illegal binary operator.\n", **p2));
    }

    return 0;
}

int
getop1 (char *cp, char **lptr)
{
    while ( *(++(*lptr)) == ' ');
    
    switch (*lptr[0])
    {
        case '!':
        case '~':
        case '+':
        case '-':
            cp[0] = *lptr[0];
            cp[1] = '\0';
            return 2;
            break;
    }

    --(*lptr);
    return 0;
}

/* ******************************************* *
 * tstnxtop() -                                *
 * fixed                                       *
 * ******************************************* */

/*tstnxtop (t_312, t_260) */
int
tstnxtop (char *p1, char **cp)
{
    char **st_266;
    char *st_264;
    char st_6[258];
    int st_4;
    int st_2;
    int st_0;

    st_264 = *cp;
    st_266 = &st_264;
    
    st_4 = getopr (st_6, st_266);
    
    if (st_4 == 3)
    {
        st_2 = opval (p1);
        st_0 = opval (st_6);
        
        if ( st_0 > st_2)
        {
            return (1);
        }
        else
        {
            return (0);
        }
    }
    
    return (0);
}

/* ************************************************** *
 * opval() - should be correct                        *
 * ************************************************** */

int
opval (char *pt)
{
    switch (*pt)
    {
        case '\0':
            return (0);
            break;
        case '|':
            if ((pt)[1] == '|')
            {
                return (1);
            }
            else
            {
                if (pt[1] == '\0')
                {
                    return (3);
                }
            }

            exit (_errmsg (0, "%s Illegal operator\n", pt));
            break;
        case '&':
            if (pt[1] == '&')
            {
                return 2;
            }
            else
            {
                if (pt[1] == '\0')
                {
                    return 5;
                }
            }

            exit (_errmsg (0, "%s Illegal operator\n", pt));
            break;
        case '^':
            if (pt[1] == 0)
            {
                return 4;
            }

            exit (_errmsg (0, "%s Illegal operator\n", pt));
            break;

        case '=':
        case '!':
            if ((pt[1] == '=') && (pt[2] == '\0'))
            {
                return 6;
            }
            exit (_errmsg (0, "%s Illegal operator\n", pt));

            break;
        case '<':
        case '>':
            if (((pt[1] == '=') && (pt[2] == '\0')) || (pt[1] == '\0'))
            {
                return 7;
            }
            else
            {
                if ((pt[1] == pt[0]) && (pt[2] == '\0'))
                {
                    return 8;
                }
            }

            exit (_errmsg (0, "%s Illegal operator\n", pt));
            break;
        case '+':
        case '-':
            return 9;
            break;
        case '*':
        case '/':
        case '%':
            return 10;
            break;
    }

    exit (_errmsg (0, "%d Unrecognized operator!\n", *pt));
}

int
calc1 (int p1, char p2)
{
    switch (p2)
    {
        case '!':
            return ((p1 == 0) ? 1 : 0);

            break;
        case '~':
            return (~p1);
            break;
        case '+':
            return (p1);
            break;
        case '-':
            return (-p1);
            break;
    }

    exit (_errmsg (0, "%d  Illegal operator.\n", p2));
}

/* ********************************************** *
 * calc2() -                                      *
 * fixed                                          *
 * ********************************************** */

int
calc2 (unsigned int p1, unsigned int p2, char *p3)
{
    switch (*p3)
    {
        case '|':
            if (p3[1] == '|')
            {
                return (((p1 != 0) || (p2 != 0)) ? 1 : 0);
            }
            else
            {
                if (p3[1] == '\0')
                {
                    return (p1 | p2);
                }
            }
            
            exit (_errmsg (0, "%s Illegal operator\n", p3));
            break;
        case '&':
            if (p3[1] == '&')
            {
                return (((p1 != 0) && (p2 != 0)) ? 1 : 0);
            }
            else
            {
                if (p3[1] == '\0')
                {
                    return (p1 & p2);
                }
            }

            exit (_errmsg (0, "%s Illegal operator\n", p3));
            break;
        case '^':
            if (p3[1] == '\0')
            {
                return (p1 ^ p2);
            }
            exit (_errmsg (0, "%s Illegal operator\n", p3));
            break;
        case '=':
            if ((p3[1] == '=') && (p3[2] == '\0'))
            {
                return (p1 == p2 ? 1 : 0);
            }

            exit (_errmsg (0, "%s Illegal operator\n", p3));
            break;
        case '!':
            if ((p3[1] == '=') && (p3[2] == '\0'))
            {
                return (p1 != p2 ? 1 : 0);
            }
            exit (_errmsg (0, "%s Illegal operator\n", p3));
            break;
        case '<':
            if ((p3[1] == '=') && (p3[2] == '\0'))
            {
                return ((p1 <= p2) ? 1 : 0);
            }
            else
            {
                if ((p3[1] == p3[0]) && (p3[2] == '\0'))
                {
                    return (p1 << p2);
                }
                else
                {
                    if (p3[1] == '\0')
                    {
                        return ((p1 < p2) ? 1 : 0);
                    }
                }
            }
            
            exit (_errmsg (0, "%s Illegal operator\n", p3));
            break;
        case '>':
            if ((p3[1] == '=') && (p3[2] == '\0'))
            {
                return (p1 >= p2 ? 1 : 0);
            }
            else
            {
                if ((p3[1] == p3[0]) && (p3[2] == '\0'))
                {
                    return (p1 >> p2);
                }
                else
                {
                    if (p3[1] == '\0')
                    {
                        return ((p1 > p2) ? 1 : 0);
                    }
                }
            }

            exit (_errmsg (0, "%s Illegal operator\n", p3));
            break;
        case '+':
            return (p1 + p2);
            break;
        case '-':
            return (p1 - p2);
            break;
        case '*':
            return (p1 * p2);
            break;
        case '/':
            return (p1 / p2);
            break;
        case '%':
            return (p1 % p2);
            break;
    }

    exit (_errmsg (0, "%d  Illegal operator.\n", p3));
}

