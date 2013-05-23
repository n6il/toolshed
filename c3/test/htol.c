/* Test: htol.c

	Simple test of htol
*/

#include <stdlib.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *a = "eadbeef";
	char *b = "0";
	char *c = "f";
	char *d = "23DE4";
	char *e = "ffffffff";
	char *f = "26";
	
	if( htol( a ) != 246267631l )
		return 10;

	if( htol( b ) != 0l )
		return 20;

	if( htol( c ) != 15l )
		return 30;

	if( htol( d ) != 146916l )
		return 40;

	if( htol( e ) != -1l )
		return 50;

	if( htol( f ) != 38l )
		return 60;

	return 0;
}