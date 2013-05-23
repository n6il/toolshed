/* Test: strncpy.c

	Simple test of strncpy
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *a = "This is a long string";
	char *b = "short";
	char *c = "abcde";
    char *d;
    
    d = strncpy( a, b, 10 );
    
    if( d != a )
    	return 10;
    
    if( a[0] != 's' )
    	return 20;
    
    if( a[5] != NULL )
    	return 30;
    
    if( a[6] != NULL )
    	return 40;
    	
    d = strncpy( a, c, 3 );
    
    if( d != a )
    	return 40;
    
    if( a[3] != 'r' )
    	return 50;
    	
	return 0;
}