/* Test: strlen.c

	Simple test of strlen
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *a = "a long\0string";
	char *b = "a short";
	char *d = "";
    int c;
    
	c = strlen( a );
	
	if( c != 6 )
		return 10;
	
	c = strlen( b );
	
	if( c != 7 )
		return 20;
	
	c = strlen( d );
	
	if( c != 0 )
		return 30;

	return 0;
}
