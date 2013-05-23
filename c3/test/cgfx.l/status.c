/* status.c - testing cgfx status functions */

#include <lowio.h>

int main( argc, argv )
int argc;
char *argv[];
{
	int i;
	int x, y, z;
	char buf[16];
	char s[255];
	
/*_Flush(); /* Turn on buffering */

	i = _gs_crsr( STDOUT, &x, &y );
	
	printf( "_gs_crsr: result %d, x %d, y %d\n", i, x, y );
	i = _gs_scsz( STDOUT, &x, &y );
	printf( "_gs_scsz: result %d, x %d, y %d\n", i, x, y );
	
	_gs_palt( STDOUT, buf );
	_dump( "after _gs_palt", buf, 16 );
	
	i = _gs_styp(STDOUT,&x);
	printf( "_gs_styp: result %d, x %d\n", i, x );

	i = _gs_fbrg(STDOUT,&x,&y,&z);
	printf( "_gs_fbrg: result %d, fore %d, back %d, bord %d\n", i, x, y, z );
	
	i = _ss_dfpl(STDOUT,buf);
	printf( "_ss_dfpl: result %d\n", i );

	i = _ss_mtyp(STDOUT,1);
	printf( "_ss_mtyp: result %d\n", i );

	return 0;
}