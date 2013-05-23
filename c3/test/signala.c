/* Test: signala.c 

	Simple test of the signal() system call.
	
	It forks a program called killvictim with a 'S' parameter.
	Killvictim will notice the S parameter and install a singal
	handler for signal 10. This program then procedes to signal
	killvictim twice with signal 10. This causes kill victim to
	exit with a status code of 40. This program will wait for a
	child process to die, makes sure it is the proccess we started.
	Then checks the result code to see if it returned 40.
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
	char *paramptr = "S\n";
	int type = 0x01;
	int lang = 0x01;
	int datasize = 0;
	int	pid, kpid;
	int result;
	
	pid = os9fork( modname, paramsize, paramptr, type, lang, datasize );
	
	/* Wait for a few ticks */
	result = tsleep( 10 );
	
	if( result == -1 )
		return 10;

	result = kill( pid, 10 );
	
	if( result )
		return 20;
	
	result = tsleep( 10 );
	
	if( result == -1 )
		return 30;

	result = kill( pid, 10 );
	
	if( result )
		return 40;
	
	kpid = wait( &result );
	
	if( kpid != pid )
		return 50;
		
	if( result != 40 )
		return 60;
	
	return 0;
	
}



