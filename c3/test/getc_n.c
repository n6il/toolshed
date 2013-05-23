/* Test: getc.c

	Simple test of getc()
*/

#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
	char c;
	
	do
	{
		c = getc( stdin );
		
		putc( c, stdout );
	} while( c != 0x0d );
	
	return 0;
}