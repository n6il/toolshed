/*
 * findstr() -- returns the position of the string pat in lin starting
 * search at position i (first position 1); returns 0 if not found.
 */

int
findstr(i, lin, pat)
register int	i;
register char	*lin, *pat;
{
	for (lin += i - 1; *lin; lin++, ++i) {
		if (smatch(lin++, pat))
			return i;
	}
	return 0;
}

/*
 * findnstr() -- returns the position of the string pat in lin starting
 * search at position i (first position 1); returns 0 if not found.
 * continues even past null bytes in lin until len position.
 */

int
findnstr(i, lin, pat, len)
register int	i;
register char	*lin, *pat;
{
	for (lin += i - 1; i <= len; lin++, i++) {
		if (smatch(lin++, pat))
			return i;
	}
	return 0;
}

/*
 * smatch() -- returns 1 if pat matches lin for the length of pat,
 * else 0
 */
static int
smatch(lin, pat)
register char	*lin, *pat;
{
	while (*pat) {
		if (*lin++ != *pat++)
			return 0;
	}
	return 1;
}
