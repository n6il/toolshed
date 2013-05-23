#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* This program will convert a Color Computer double to a GCC double then
   display the result.


	 gcc double: seeeeeeeeeeeffffffffffffffffffffffffffffffffffffffffffffffffffff, bias 1023
	coco double: sfffffffffffffffffffffffffffffffffffffffffffffffffffffffeeeeeeee, bias 128
	             0123456701234567012345670123456701234567012345670123456701234567
*/

int main( int argc, char *argv[] )
{	
	int i, commas = 0;
	char *p;
	
	union
	{
		double x;
		unsigned char c[8];
		unsigned short s[4];
		unsigned int i[2];
		unsigned long l[2];
		unsigned long long lp;
	} a;
	
	unsigned long long sign, ue;
	long long exponent;

	
	if( sizeof(a) != 8 )
	{
		printf ("Compile time error: sizeof a != 8\n" );
		return 0;
	}
	
	if( argc != 2 )
	{
		printf( "I need 1 argument: %s 3423,32432,2343,34234\n", argv[0] );
		return 0;
	}
	
	/* count the number of commas */
	
	for( i=0; i<strlen(argv[1]); i++ )
	{
		if( argv[1][i] == ',' )
			commas++;
	}
	
	switch( commas )
	{
		case 0:
			a.lp = strtoll(argv[1], NULL, 0);
			break;
		
		case 1:
			a.l[0] = strtol( argv[1], &p, 0 );
			p++;
			a.l[1] = strtol( p, NULL, 0 );
			break;
		
		case 3:
			a.s[0] = strtol( argv[1], &p, 0 );
			p++;
			a.s[1] = strtol( p, &p, 0 );
			p++;
			a.s[2] = strtol( p, &p, 0 );
			p++;
			a.s[3] = strtol( p, NULL, 0 );
			p++;
			break;
		
		case 7:
			a.c[0] = strtol( argv[1], &p, 0 );
			p++;
			a.c[1] = strtol( p, &p, 0 );
			p++;
			a.c[2] = strtol( p, &p, 0 );
			p++;
			a.c[3] = strtol( p, &p, 0 );
			p++;
			a.c[4] = strtol( p, &p, 0 );
			p++;
			a.c[5] = strtol( p, &p, 0 );
			p++;
			a.c[6] = strtol( p, &p, 0 );
			p++;
			a.c[7] = strtol( p, NULL, 0 );
			break;
		default:
			printf( "Wrong nuber of commas in argument\n" );
			return 0;
			break;
	}

	/* Convert CoCo Double to GCC Double */
	sign = a.c[0] & 0x80 ? 1 : 0;

	exponent = a.c[7];	
	exponent -= 130;
	ue = exponent + 1024;

	a.lp = a.lp >> 11;
	a.lp |= ue << 52;
	a.lp |= sign << 63;
	
	printf( "value = %f\n", a.x );

	return 0;
}
