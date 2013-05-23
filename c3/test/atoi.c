/* Test: atoi.c

	Simple test of atoi
*/
#include <stdlib.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *s1 = "-20";
	char *s2 = "  20ksjdslask";
	char *s3 = "+32554.23";
	char *s4 = "akm32kkj";
	
	
	if( atoi( s1 ) != -20 )
		return 10;
		
	if( atoi( s2 ) != 20 )
		return 20;
		
	if( atoi( s3 ) != 32554 )
		return 30;
		
	if( atoi( s4 ) != 0 )
		return 40;
		
	return 0;
}