/* Test: system.c

	Simple test of reverse
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *a = "echo test";
	char *b = "argv kj"; /* Sort of a recursive test. */
	int	c;
	
	c = system( a );
	
	if( c != 0 )
		return 10;
		
	c = system( b );
	
	if( c != 10 )
		return 10;
		
	return 0;
}