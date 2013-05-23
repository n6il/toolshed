/* Test: gets.c

	Simple test of gets()
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(argc,argv)
int argc;
char **argv;
{
	char s[256];
	
	gets( s );

	return 0;
}