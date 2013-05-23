/* Test: clshifts.c 

	Simple test of the lng shifts.
*/

int main(argc,argv)
int argc;
char **argv;
{
	long i=8;
	long b=2;
	long c=-2;
	int x=2;

	if( i>>b != 2 )
		return 10;

	if( i<<b != 32 )
		return 20;
		
	if( i>>c != 0 )
		return 30;
		
	if( i<<c != 0 )
		return 40;
		
	if( i>>x != 2 )
		return 50;
		
	if( i<<x != 32 )
		return 60;
		
	return 0;
}