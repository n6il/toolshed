/* Test: l3tol.c

	Simple test of l3tol
*/

int main(argc,argv)
int argc;
char **argv;
{
	char a[6];
	long l[2];
	
	a[0] = 0x11;
	a[1] = 0x22;
	a[2] = 0x33;
	a[3] = 0x22;
	a[4] = 0x33;
	a[5] = 0x44;
	
	l3tol( l, a, 2 );
	
	if( l[0] != 1122867l )
		return 10;
		
	if( l[1] != 2241348l )
		return 10;
		
	return 0;
}