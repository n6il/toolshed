/* Test: clconvert.c 

	Simple test of the lng converting.
*/

int main(argc,argv)
int argc;
char **argv;
{
	long l=27, o=-27;
	long a=4, b = -4;

	if( (l/a) != 6l )
		return 10;
	
	if( (l/b) != -6l )
		return 20;
	
	if( (o/a) != -6l )
		return 30;
	
	if( (o/b) != 6l )
		return 40;
	

	if( (l%a) != 3l )
		return 50;
	
	if( (l%b) != 3l )
		return 60;
	
	if( (o%a) != -3l )
		return 70;
	
	if( (o%b) != -3l )
		return 80;


	if( (a/l) != 0l )
		return 90;
	
	if( (b/l) != 0l )
		return 100;
	
	if( (a/o) != 0l )
		return 110;
	
	if( (b/o) != 0l )
		return 120;
	

	if( (a%l) != 4l )
		return 130;
	
	if( (b%l) != -4l )
		return 140;
	
	if( (a%o) != 4l )
		return 150;
	
	if( (b%o) != -4l )
		return 160;

	return 0;
}