/*
 * mktemp()
 * Provides a unique name for temporary files.
 * The user must supply a pointer to a string whose last 5 characters
 * are 'X'; the Xs are replaced with the ascii representation of
 * the process id. Returns its pointer argument.
 */
 
char *
mktemp(s)
char	*s;
{
	register char	*p;
	int				c;

	for (p = s; (c = *p) != '\0'; ++p) {
		if (c == 'X') {
			sprintf(p, "%d", getpid());
			break;
		}
	}
	return s;
}
