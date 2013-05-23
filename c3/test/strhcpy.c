/* Test: strhcpy.c

	Simple test of strtok
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *a = "                ";
	char *b = "I am usual";
    char *d;
    
    /* Create OS-9 string */
    
    b[9] |= 0x80;
    
    /* Copy OS-9 string to c string */
    d = strhcpy( a, b );
    
    if( d != a )
    	return 10;
    	
    if( a[9] != 'l' )
    	return 20;
    
    if( a[10] != NULL )
    	return 30;

	return 0;
}