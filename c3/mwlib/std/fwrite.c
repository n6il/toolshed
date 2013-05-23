#include <stdio.h>

int
fwrite(ptr, size, n, fp)
register char	*ptr;
int				size, n;
FILE			*fp;
{
	int				tn;
	register int	ts;

	for (tn = 0; tn < n; ++tn) {
		for (ts = 0; ts++ < size;) {
			putc(*ptr++, fp);
			if (ferror(fp))
				return tn;
		}
	}
	return tn;
}
