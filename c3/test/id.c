/* Test: id.c 

	Simple test of the id.a calls.
*/

#include <os9.h>

int main(argc,argv)
int argc;
char **argv;
{
	struct registers regs;
	int	result;

	result = _os9( F_ID, &regs );

	if( result )
		return 10;

	if( getpid() != regs.rg_a )
		return 20;
	
	if( getuid() != regs.rg_y )
		return 30;
	
	result = setuid( 10 );
	
	if( result )
		return 40;

	if( getuid() != 10 )
		return 50;

	result = _os9( F_ID, &regs );

	if( result )
		return 60;
		
	if( getuid() != regs.rg_y )
		return 70;

	return 0;
}