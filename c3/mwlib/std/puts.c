#include <stdio.h>

puts(s)
char	s[];
{
	fputs(s, stdout);
	putchar('\n');
}

fputs(s, fp)
register char	*s;
register FILE	*fp;
{
	char	c;

	while (c = *s++)
		putc(c, fp);
}

