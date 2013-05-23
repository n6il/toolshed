/* Test: uminmax.c 

	Simple test of the umin and umax.
*/

int main(argc,argv)
int argc;
char **argv;
{
	unsigned l=27, o=64;

	if( umin( l, o ) != 27 )
		return 10;
		
	if( umin( o, l ) != 27 )
		return 20;

	if( umax( l, o ) != 64 )
		return 30;
		
	if( umax( o, l ) != 64 )
		return 40;

	return 0;
}