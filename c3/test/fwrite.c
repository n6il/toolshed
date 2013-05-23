/* Test: fwrite.c

	Simple test of fwrite
*/

#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
	char a[3][20];
	
	memset( a, 0x97, 20*3 );
	
	strcpy( a[0], "Line one" );
	strcpy( a[1], "Line two" );
	strcpy( a[2], "Line three" );
	
	a[0][19] = 0x0d;
	a[1][19] = 0x0d;
	a[2][19] = 0x0d;

	fwrite( a, 20, 3, stdout );
	
	return 0;
}