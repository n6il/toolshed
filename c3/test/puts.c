/* Test: putc.c

	Simple test of put.c
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *a = "Match this puts.";
	
	fputs( a, stdout );
	fputs( "\n", stdout );
	puts( a );
	
	return 0;
}