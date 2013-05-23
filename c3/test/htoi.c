/* Test: htoi.c

	Simple test of htoi
*/

#include <stdlib.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *a = "0";
	char *b = "1";
	char *c = "f";
	char *d = "1e";
	char *e = "ffff";
	char *f = "7fff";
	
	if( htoi( a ) != 0 )
		return 10;

	if( htoi( b ) != 1 )
		return 20;

	if( htoi( c ) != 15 )
		return 30;

	if( htoi( d ) != 30 )
		return 40;

	if( htoi( e ) != -1 )
		return 50;

	if( htoi( f ) != 32767 )
		return 60;

	return 0;
}