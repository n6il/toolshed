/* Test: stata.c 

	Simple test of the getstat and setstat() system call.
*/

#define os9in 0
#define os9out 1
#define os9err 2

#include <os9.h>
#include <sgstat.h>
#include <direct.h>

int main(argc,argv)
int argc;
char **argv;
{
	char *scf = "/Term\n";
	char *rbf = "/dd/startup\n";
	char name[32];
	struct sgbuf buf;
	int path, result;
	long size, seek, pos;
	struct fildes buf2;

	path = open( scf, 1 );
	
	if( path == -1 )
		return 10;
		
	result = getstat(SS_OPT, path, &buf);
	
	if( result == -1 )
		return 20;

	if( buf.sg_class != 0 )
		return 30;
	
	result = getstat(SS_READY, path);

	if( result != -1 )
		return 40;
	
	close( path );
	
	path = open( rbf, 1 );
	
	if( path == -1 )
		return 50;
		
	result = getstat(SS_SIZE, path, &size);
	
	if( size < 100 )
		return 60;
		
	seek = lseek( path, 2l, 0 );

	if( seek == -1l )
		return 70;
	
	if( seek != 2l )
		return 80;
	
	result = getstat(SS_POS, path, &pos);
	
	if( result == -1 )
		return 90;
	
	if( pos != 2 )
		return 100;
	
	result = getstat(SS_EOF, path);
	
	if( result == -1 )
		return 110;
	
	result = getstat(SS_DEVNM, path, name);
	
	if( result == -1 )
		return 120;
	
	if( name[1] == 'd' )
		return 130;

	result = getstat(SS_FD, path, &buf2, sizeof(buf2));
	
	if( result == -1 )
		return 140;
		
	if( buf2.fd_fsize < 100 )
		return 150;
		
	close( path );
	
	return 0;
}
