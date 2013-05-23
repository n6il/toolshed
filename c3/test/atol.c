/* Test: atol.c

	Simple test of atol
*/
#include <stdlib.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *s1 = "-20";
	char *s2 = "  20ksjdslask";
	char *s3 = "+2241348.23";
	char *s4 = "akm32kkj";
	
	
	if( atol( s1 ) != -20l )
		return 10;
		
	if( atol( s2 ) != 20l )
		return 20;
		
	if( atol( s3 ) != 2241348l )
		return 30;
		
	if( atol( s4 ) != 0l )
		return 40;
		
	return 0;
}