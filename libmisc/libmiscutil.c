/********************************************************************
 * util.c - Functions common to all utilities
 *
 * $Id$
 ********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <util.h>
#include <cococonv.h>
#include <cocopath.h>
#include <cocotypes.h>


int StrToInt(char *s)
{
    int accum = 0;
    int i, t, power;

    switch (*s)
    {
        case '$':	/* Hex conversion */
            s++;
            t = strlen(s);
            for (i = 0; i < t; i++)
            {
                int c;

                c = toupper(s[i]);
                if (c >= 'A' && c <= 'F')
                {
                    c -= 7;
                }

                c -= 48;
                power = pow(16, t - i - 1);	

                accum += c * power;
            }
            break;

        case '%':	/* Binary conversion */
            s++;
            t = strlen(s);
            for (i = 0; i < t; i++)
            {
                int c;

                c = s[i];
                c -= 48;
                power = pow(2, t - i - 1);	
                accum += c * power;
            }
            break;

        case '0':	/* Octal conversion */
            s++;
            t = strlen(s);
            for (i = 0; i < t; i++)
            {
                int c;

                c = s[i];
                c -= 48;

                power = pow(8, t - i - 1);	
                accum += c * power;
            }
            break;


        default:	/* Decimal conversion */
            accum = atoi(s);
            break;
    }

    return(accum);	
}


#ifdef BDS
int strcasecmp(char *s1, char *s2)
{
	while (*s1 != '\0' && *s2 != '\0')
	{
		if (*s1 == *s2)
		{
			s1++;
			s2++;
			continue;
		}
		if (*s1 > *s2)
		{
			return -1;
		}
		if (*s1 < *s2)
		{
			return 1;
		}
	}

	if (*s1 == '\0' && *s2 == '\0')
	{
		return 0;
	}
	if (*s1 == '\0')
	{
		return -1;
	}

	return 1;
}
#endif


void show_help(char **helpMessage)
{
    char **p = helpMessage;

    while (*p)
    {
        fputs(*(p++), stderr);
    }

    return;
}

