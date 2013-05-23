/* Test: clnegcomp.c 

	Simple test of the long neg and complementing.
*/

int main(argc,argv)
int argc;
char **argv;
{
	long l = 16, o = -16;
	
	if( -l != -16l )
		return 10;
	
	if( ~l != -17l )
		return 20;
	
	if( -o != 16l )
		return 30;
	
	if( ~o != 15l )
		return 40;
		
	if( -l != -16 )
		return 50;
	
	if( ~l != -17 )
		return 60;
	
	if( -o != 16 )
		return 70;
	
	if( ~o != 15 )
		return 80;
		
	return 0;
	
}