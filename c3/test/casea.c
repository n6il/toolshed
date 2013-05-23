/* Test: casea.c

	Simple test of to upper and tolower
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *a = "\000\000\001\002\003\010adehjfiiuodAKSUSHRNFMUI@#$%%^&*()\205\220";
	char *b = "\000\000\001\002\003\010ADEHJFIIUODAKSUSHRNFMUI@#$%%^&*()\205\220";
	char *c = "\000\000\001\002\003\010adehjfiiuodaksushrnfmui@#$%%^&*()\205\220";
	int i;
	
	for( i=1; i<41; i++ )
	{
		if( toupper(a[i]) != b[i] )
			return i;
	}
	
	for( i=1; i<41; i++ )
	{
		if( tolower(a[i]) != c[i] )
			return 64+i;
	}
	
	return 0;
}