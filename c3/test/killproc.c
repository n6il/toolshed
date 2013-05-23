/* Test: kill.c 

	Simple test of the kill() system call.
*/

#include <os9.h>
#include <signal.h>

int main(argc,argv)
int argc;
char **argv;
{
	struct registers regs;
	char *modname = "killvictim\n";
	int paramsize = 2;
	char *paramptr = "K\n";
	int type = 0x01;
	int lang = 0x01;
	int datasize = 0;
	int	pid, kpid;
	int result;

	pid = os9fork( modname, paramsize, paramptr, type, lang, datasize );
	
	if( pid == -1 )
		return 10;
		
	/* Wait for a few ticks */
	result = tsleep( 10 );
	
	if( result == -1 )
		return 20;

	result = kill( pid, SIGQUIT );
	
	if( result )
		return 30;
	
	kpid = wait( &result );
	
	/* SIGKILL is the proper result of killvictim */
	if( result != SIGQUIT )
		return 40;
	
	if( kpid != pid )
		return 50;
		
	return 0;
	
}



