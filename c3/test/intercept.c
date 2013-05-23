/* Test: intercept.c 

	Simple test of the intercept() system call.
*/

#include <os9.h>

int pn;
int intrupt();

int main(argc,argv)
int argc;
char **argv;
{
	struct registers regs;
	int i;
	int	result;
	
	pn = 10;
	
	intercept( intrupt );
	
	/* Get our process ID */
	result = _os9( F_ID, &regs );

	if( result )
		return 10;
		
	regs.rg_b = pn;
	
	/* Send signal to our self */
	result = _os9( F_SEND, &regs );
	
	if( result )
		return 20;

	/* Wait for a few ticks */
	
	result = tsleep( 10 );
	
	if( result == -1 )
		return 30;

	/* If we got the signal, return success */
	if( pn == 30 )
		return 0;
		
	return 1;
}

int intrupt( sig )
int sig;
{
	/* Tell main procesess we've been signaled */

	if( sig == 10 )
		pn = 30;
}

