#ifndef M6809
#include <stdio.h>
#include <string.h>

/*
 * Looks for a string inside another one and returns a pointer to its
 * occurence or else a NULL.
 */
int             findstr(pos, line_str, string)
	int             pos;
	char           *line_str,
	               *string;
{
	char           *cp;

	line_str = line_str + pos - 1;

	for (cp = line_str; (*cp != '\0'); cp++)
	{
		cp = strchr(cp, *string);	/* find the first char. */
		if (cp == NULL)
			return (0);
		else if (strncmp(cp, string, strlen(string)) == 0)
			return (cp - line_str);
	}

	return (0);
}

#else
#asm
findstr:
pshs u, y
ldd 6, s(pos)
addd 8, s(string)
subd
#1
tfr d, y(string + pos)
ldu 10, s(pattern)
tst, u
beq NMATCH
LP2
lda, y +
beq NMATCH
cmpa, u
bne LP2
leax, y
leau 1, u
LP3
tst, u
beq MATCH2
lda, y +
beq NMATCH
cmpa, u +
beq LP3
ldu 10, s
leay, x
bra LP2
MATCH2
tfr x, d
subd 8, s
puls u, y, pc
NMATCH
clra
clrb
puls u, y, pc
#endasm
#endif
