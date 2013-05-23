/* Test: clbits.c 

	Simple test of the long bit ops.
*/

int main(argc,argv)
int argc;
char **argv;
{
	long l = 27, o = -27;
	long a = 1, b = -1;
	
	if( l & a != 1l )
		return 10;

	if( (l | a) != 27l )
		return 20;
		
	if( (l ^ a) != 26l )
		return 30;
	
	if( (!l) != 0 )
		return 40;
		
	if( (l & b) != 27l )
		return 50;
	
	if( (l | b) != -1l )
		return 60;
		
	if( (l ^ b) != -28l )
		return 70;
	
	if( (!l) != 0 )
		return 80;

		
	if( (o & a) != 1l )
		return 90;
	
	if( (o | a) != -27l )
		return 100;
		
	if( (o ^ a) != -28l )
		return 110;
	
	if( (!o) != 0 )
		return 120;
		
	if( (o & b) != -27l )
		return 130;
	
	if( (o | b) != -1l )
		return 140;
		
	if( (o ^ b) != 26l )
		return 150;
	
	if( (!o) != 0 )
		return 160;
		
	return 0;
	
}