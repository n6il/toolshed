/* Test: setjmp.c 

	Simple test of setjmp, longjmp.
*/

#include <setjmp.h>

jmp_buf	env;

int main(argc,argv)
int argc;
char **argv;
{
	int	result;
	int i;
	int a=5, b=6;

	result = setjmp( env );
	
	if( result == 0 )
	{
		if( do_some( &i ) == 0 )
		{
			return 10;
		}
		
		return 5;
	}
	else if( result == 63 )
	{
		if( i == 59 )
		{
			if( a != 5 )
				return 5;
			
			if( b != 6 )
				return 6;
				
			return 0;
		}
		else
			return 20;
	}
	
	if( i != 59 )
		return 30;	
	
	return 40;
}

int do_some( i )
int *i;
{
	int a;
	
	for( a=0; a<100; a++ )
	{
		if( a == 59 )
		{
			*i = a;
			longjmp( env, 63 );
			return 60;
		}
	}
	
	return 0;
}
