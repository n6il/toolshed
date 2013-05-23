/* Test: swab.c

	Simple test of swab()
*/

#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
	int i = 0xabbc;
	
	i = swab(i);
	
	if( i != 0xbcab )
		return 10;
	
	return 0;
}