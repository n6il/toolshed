/* Test: strpbrk.c

	Simple test of strpbrk
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *a = "aaabbbcczyx";
	char *b = "jki";
	char *c = "xyz";
    char *d;
    
	d = strpbrk( a, b );
	
	if( d != NULL )
		return 10;
		
	d = strpbrk( a, c );
	
	if( d != a+8 )
		return 20;

	return 0;
}