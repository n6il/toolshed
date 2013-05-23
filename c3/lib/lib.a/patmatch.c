#include <ctype.h>

patmatch(p, s, f)
char *p;                          /* pattern */
register char *s;                 /* string to match */
char f;                           /* flag for case force */
{
	char   pc;                    /* a single character from pattern */

	while (pc = (f ? toupper(*p++) : *p++))
	{
		if (pc == '*')
		{
			do
			{
				if (patmatch (p, s, f))
					return (1);
			}
            while (*s++);

            return 0;
		}
         else
            if (!*s)
            	return (0);
			else if (pc == '?')
				s++;
			else if (pc != (f ? toupper(*s++) : *s++))
				return 0;
	}

	return !*s;
}
