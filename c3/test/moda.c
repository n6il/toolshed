/* Test: moda.c

	Simple test of the mod_a calls.
*/

#include <module.h>

int main(argc,argv)
int argc;
char **argv;
{
	mod_exec *mod;
	char *modname = "shell";
	char *modpath = "/dd/CMDS/echo";
	int	result;

	mod = modlink( modname, 1, 1 );
	
	if( mod == -1 )
		return 10;
	
	if( mod->m_sync != 0x87cd )
		return 20;
	
	result = munlink( mod );
	
	if( result == -1 )
		return 30;
	
	mod = modload( modpath, 1, 1 );
	
	if( mod == -1 )
		return 40;
	
	if( mod->m_sync != 0x87cd )
		return 50;
	
	result = munlink( mod );
	
	if( result == -1 )
		return 60;

	return 0;
}