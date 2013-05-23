/* Test: mema.c 

	Simple test of the mem_a library functions.
*/

#include <os9.h>

int main(argc,argv)
int argc;
char **argv;
{
	struct registers regs;
	int result;
	int	orginal_size;
	int	new_size;
	char *newsRam;
	char *newiRam;
	
	regs.rg_a = 0;
	regs.rg_b = 0;
	
	result = _os9( F_MEM, &regs );
	
	if( result )
		return 10;
	
	orginal_size = *(short *)&(regs.rg_a);

	newsRam = sbrk( 512 );
	
	if( newsRam == -1 )
		return 20;
	
	newiRam = ibrk( 256 );
	
	if( newiRam == -1 )
		return 30;
	
	result = unbrk( orginal_size );
	
	if( result == -1 )
		return 40;
		
	regs.rg_a = 0;
	regs.rg_b = 0;
	
	result = _os9( F_MEM, &regs );

	if( result )
		return 50;
		
	new_size = *(short *)&(regs.rg_a);

	if( new_size != orginal_size )
		return 60;

	return 0;
	
}



