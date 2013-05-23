/* Test: skip.c

	Simple test of skip()
	
*/

char *skipbl();
char *skipwd();

#include <ctype.h>
#include <sets.h>
#include <local.h>
#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *a = "   \t This is the first word\n";
	char *b;
	char *c = "Find the next word";
	
	b = skipbl( a );
	
	if( b != a+5 )
		return 10;
		
	b = skipwd( c );
	
	if( b != c+4 )
		return 20;
	
	b = skipbl( b );
	
	if( b != c+5 )
		return 30;
		
	return 0;	
}
