/* Test: qsorta.c 

	Simple test of qsort.
*/

int compar();
extern char *bsearch();

int main(argc,argv)
int argc;
char **argv;
{
	int a[5];
	int *b;
	int key;
	
	a[0] = 2;
	a[1] = 1;
	a[2] = 5;
	a[3] = 3;
	a[4] = 4;

	qsort( a, 5, 2, compar );
	
	key = 3;
	
	b = bsearch( &key, a, 5, sizeof( int ), compar );

	if (*b != 3 )
		return 10;
		
	key = 5;
	b = bsearch( &key, a, 5, sizeof( int ), compar );
	
	if (*b != 5 )
		return 20;
		
	key = 1;
	b = bsearch( &key, a, 5, sizeof( int ), compar );
	
	if (*b != 1 )
		return 30;

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