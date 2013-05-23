#include <stdio.h>
#include <modes.h>

static int	prcid;
FILE		*fdopen();

FILE *
popen(command, type)
char	*command, *type;
{
	int	dupe, pipe, stdpath;

	if (prcid)
		return 0;

	switch (*type) {
	case 'r':
		stdpath = 1;
		break;
	case 'w':
		stdpath = 0;
		break;
	default:
		return 0;
	}

	if ((pipe = open("/pipe", S_IWRITE | S_IREAD)) == -1)
		return 0;

	/* dup std path */
	if ((dupe = dup(stdpath)) == -1) {
		close(pipe);
		return 0;
	}

	close(stdpath);		/* close std path */
	dup(pipe);			/* hook pipe to stdout */
	if ((prcid = os9fork("shell", strlen(command), command, 1, 1, 0)) == -1) {
		close(pipe);	/* close pipeline */
		dup(dupe);		/* restore std path */
		return 0;
	}
	close(stdpath);				/* close pipe */
	dup(dupe);					/* restore std path */
	close(dupe);				/* close stdout copy */
	return fdopen(pipe, type);	/* get fp for pipeline */
}

pclose(fp)
FILE	*fp;
{
	int		s;

	if (!prcid)
		return -1;

	fclose(fp);
	while (wait(&s) != prcid)
		;
	prcid = 0;
	return s;
}
