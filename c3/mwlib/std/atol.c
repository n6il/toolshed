#include <ctype.h>

#define TRUE	1
#define FALSE	0

/*
 * atol() -- returns the long integer value expressed in digits in
 * string s
 * any non-digit character terminates the the funtion, but
 * a leading minus sign is recognized.
 * Leading spaces and/or tabs are skipped.
 */

long atol(s)
register char	*s;
{
	int 	minus;
	long	n;
	char	c;

	n = 0;
	while ((c = *s++) == ' ' || c == '\t')
		;

	minus = FALSE;

	if (c == '-') {
		minus = TRUE;
		c = *s++;
	} else if (c == '+')
		c = *s++;

	while (isdigit(c)) {
		n = n * 10 + (c - '0');
		c = *s++;
	}

	return minus ? -n : n;
}
