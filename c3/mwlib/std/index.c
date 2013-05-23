/*
 * index()
 * returns a pointer to the first occurrence of char c in
 * string s or NULL if not found
 */

#define NULL 0

char *
index(s, c)
register char	*s;
int				c;
{
	for (; *s != '\0'; s++) {
		if (*s == c)
			return s;
	}
	return NULL;
}

/*
 * rindex()
 * returns a pointer to the last occurrence of char c in
 * string s or NULL if not found
 *
 * (Question: would it be faster, on the average (if that makes any
 * sense), to use
 *
 * save = NULL;
 * for (p = s; *p; p++) {
 *		if (*p == c)
 *			save = p;
 * }
 * return save;
 *
 * Clearly one can construct obnoxious cases for either method!)
 */
char *
rindex(s, c)
char	*s, c;
{
	register char	*p;

	for (p = s; *p; ++p)
		;
	while (p-- != s) {
		if (*p == c)
			return p;
	}
	return NULL;
}
