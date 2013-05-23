#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>


int lnread(path, s, n)
int path;
char *s;
int n;
{
	int count, i;


	i = 0;

	while (n-- > 0)
	{
		count = read(path, s, 1);

		if (count <= 0)
		{
			return -1;
		}


		if (*s == '\n')
		{
			*s = '\0';

			return i;
		}

		s++;
		i++;
	}
	
	
	return -1;	/* Added - BGP */
}
