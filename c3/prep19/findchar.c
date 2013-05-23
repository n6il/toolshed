#include <stdio.h>
#include <string.h>

/*
 * Looks for a string inside another one and returns a pointer to its
 * occurance or else a NULL.
 */
int             findchar(search_str, c)
	char           *search_str,
	               c;
{
	int		index = 0;

	while (search_str[index] != '\0')
	{
		if (search_str[index++] == c)
		{
			return 1;
		}
	}

	return (0);
}

