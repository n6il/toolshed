/* Test: ltol3.c

	Simple test of ltol3
*/

int main(argc,argv)
int argc;
char **argv;
{
	char a[6];
	long l[2];
	
	l[0] = 1122867l;
	l[1] = 2241348l;

	ltol3( a, l, 2 );
	
	if( a[0] != 0x11 )
		return 10;
		
	if( a[1] != 0x22 )
		return 20;
		
	if( a[2] != 0x33 )
		return 30;
		
	if( a[3] != 0x22 )
		return 40;
		
	if( a[4] != 0x33 )
		return 50;
		
	if( a[5] != 0x44 )
		return 60;
	
	return 0;
}