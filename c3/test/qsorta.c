/* Test: qsorta.c 

	Simple test of qsort.
*/

int compar();

int main(argc,argv)
int argc;
char **argv;
{
	int a[5];
	
	a[0] = 2;
	a[1] = 1;
	a[2] = 5;
	a[3] = 3;
	a[4] = 4;

	qsort( a, 5, 2, compar );
	
	if( a[0] != 1 )
		return 10;
		
	if( a[1] != 2 )
		return 20;
		
	if( a[2] != 3 )
		return 30;
		
	if( a[3] != 4 )
		return 40;
		
	if( a[4] != 5 )
		return 50;
		
	return 0;
	
}

int compar( a, b )
int *a, *b;
{
	if( *a > *b )
		return 1;
	
	if( *b > *a )
		return -1;
	
	return 0;
}