/* Test: clshifts.c 

	Simple test of the lng shifts.
*/

int main(argc,argv)
int argc;
char **argv;
{
	long i=8, o=-8;
	int a=8, b=-8;

	if( (i+o) != 0 )
		return 10;
	
	if( (o+i) != 0 )
		return 20;
	

	if( (i+b) != 0 )
		return 30;
	
	if( (b+i) != 0l )
		return 40;
	

	if( (o+b) != -16l )
		return 50;
	
	if( (b+o) != -16l )
		return 60;
		
/******/

	if( (i-o) != 16 )
		return 70;
	
	if( (o-i) != -16 )
		return 80;
	

	if( (i-b) != 16 )
		return 90;
	
	if( (b-i) != -16l )
		return 100;
	

	if( (o-b) != 0l )
		return 110;
	
	if( (b-o) != 0l )
		return 120;


	return 0;
}