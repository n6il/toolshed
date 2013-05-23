/* Test: ccmult.c 

	Simple test of the integer multiplication.
*/

int main(argc,argv)
int argc;
char **argv;
{
	int i=5;
	int b=2;
	unsigned u=5;
	unsigned x=2;
	
	if( i*b != 10 )
		return 10;
	
	if( u*x != 10 )
		return 20;

	if( i*x != 10 )
		return 30;
	
	if( u*b != 10 )
		return 40;
		
	return 0;
}