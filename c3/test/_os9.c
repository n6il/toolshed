/* Test: _os9.c 

	Simple test of the _os9() system call.
*/

#include <os9.h>

int main(argc,argv)
int argc;
char **argv;
{
	struct registers regs;
	char	*buf = "This is a buffer";
	
	union
	{
		char c[4];
		unsigned u[2];
	} crc;
	
	crc.c[0] = 0;
	crc.c[1] = crc.c[2] = crc.c[3] = 0xff;
	
	regs.rg_x = &(buf[0]);
	regs.rg_y = 16;
	regs.rg_u = &(crc.c[1]);

	_os9( F_CRC, &regs );
	
	if( crc.u[0] == 0x00b3 )
	{
		if( crc.u[1] == 0xa7cc )
		{
			return 0;
		}
	}
	
	return 1;
}