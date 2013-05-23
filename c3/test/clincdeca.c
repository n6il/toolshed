/* Test: clshifts.c 

	Simple test of the lng shifts.
*/

int main(argc,argv)
int argc;
char **argv;
{
	long i=8;

	i++;
	if( i != 9 )
		return 10;
		
	++i;
	if( i != 10 )
		return 20;
	
	i--;
	if( i != 9 )
		return 30;
	
	--i;
	if( i != 8 )
		return 40;
	
	i = -8;
	
	i++;
	if( i != -7 )
		return 50;
		
	++i;
	if( i != -6 )
		return 60;
	
	i--;
	if( i != -7 )
		return 70;
	
	--i;
	if( i != -8 )
		return 80;
		
	return 0;
}