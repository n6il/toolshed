/* Test: xtoa.c

	Simple test of utoa and itoa.
*/

int main(argc,argv)
int argc;
char **argv;
{
	int a = 10;
	int b = -10;
	int c = 0;
	int d = 32767;
	int e = -32766;
	
	unsigned f = 0;
	unsigned g = 256;
	unsigned h = 32768;
	unsigned i = 65535;
	
	char s[50];
	char *z;
	
	if( strcmp( itoa( a, s ), "10" ) != 0 )
		return 10;
	
	if( strcmp( itoa( b, s ), "-10" ) != 0 )
		return 20;
	
	if( strcmp( itoa( c, s ), "0" ) != 0 )
		return 30;
	
	if( strcmp( itoa( d, s ), "32767" ) != 0 )
		return 40;
	
	if( strcmp( itoa( e, s ), "-32766" ) != 0 )
		return 50;
	
	if( strcmp( utoa( f, s ), "0" ) != 0 )
		return 60;
	
	if( strcmp( utoa( g, s ), "256" ) != 0 )
		return 70;
	
	if( strcmp( utoa( h, s ), "32768" ) != 0 )
		return 80;
	
	if( strcmp( utoa( i, s ), "65535" ) != 0 )
		return 90;

	return 0;
}