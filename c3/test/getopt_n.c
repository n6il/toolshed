/* Test: getopt.c

	Simple test of getopt()
*/

#include <stdio.h>

main (argc, argv)
int argc;
char **argv;
{
	int c, bflg, aflg, ifile, ofile, errflg;
	extern char *optarg;
	extern int optind;

while ((c = getopt(argc, argv, "abf:o:")) != EOF)
	switch (c)
	{
		case 'a':
			if (bflg)
				errflg++;
			else
				aflg++;
			break;
		case 'b':
			if (aflg)
				errflg++;
			else
				bproc();
			break;
		case 'f':
			ifile = optarg;
			break;
		case 'o':
			ofile = optarg;
			break;
		case '?':
			errflg++;
	}

	if (errflg)
	{
		fprintf(stderr, "Usage:... \n\n");
		exit (2);
	}
	
	return 0;
}

bproc()
{
	return 0;
}