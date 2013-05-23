/* Test: minmax.c 

	Simple test of the min maxing.
*/

int main(argc,argv)
int argc;
char **argv;
{
	unsigned l=27, o=-64;

	if( min( l, o ) != -64 )
		return 10;
		
	if( min( o, l ) != -64 )
		return 20;

	if( max( l, o ) != 27 )
		return 30;
		
	if( max( o, l ) != 27 )
		return 40;

	return 0;
}