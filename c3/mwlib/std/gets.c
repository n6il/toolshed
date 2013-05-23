#include <stdio.h>

char *
gets(s)
char	s[];
{
	register char	*p;
	int				c;

	p = s;
	while ((c = getchar()) != '\n') {
		if (c == EOF)
			return NULL;
      	*p++ = c;
	}
	*p = '\0';
	return s;
}

char *
fgets(s, n, ioptr)
char			*s;
register int	n;
FILE			*ioptr;
{
	int				c;
	register char	*p;

	p = s;
	while (--n > 0) {
		if ((c = getc(ioptr)) == EOF)
			return NULL;
		if ((*p++ = c) == '\n')
			break;
	}
	*p = '\0';
	return s;
}
