/* Test: ltoa.c

	Simple test of ltoa
*/

#include <stdlib.h>

int main(argc,argv)
int argc;
char **argv;
{
	long a = 10l;
	long b = -10l;
	long c = 0l;
	long d = 1236734l;
	long e = -1236734l;
	long f = 2147483647l;
	
	char s[50];
	
	if( strcmp( ltoa( a, s ), "10" ) != 0 )
		return 10;
	
	if( strcmp( ltoa( b, s ), "-10" ) != 0 )
		return 20;
	
	if( strcmp( ltoa( c, s ), "0" ) != 0 )
		return 30;
	
	if( strcmp( ltoa( d, s ), "1236734" ) != 0 )
		return 40;
	
	if( strcmp( ltoa( e, s ), "-1236734" ) != 0 )
		return 50;
	
	if( strcmp( ltoa( f, s ), "2147483647" ) != 0 )
		return 60;
	
	return 0;
}