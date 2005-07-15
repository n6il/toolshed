#include <stdio.h>
#include <string.h>
#include "rlink.h"


static int compute_crc();
unsigned char _crc[3];


int create_decb_module()
{
	/* Write Disk BASIC BIN Header */

	/* Write Disk BASIC BIN Code/Data Segment */

	/* Write Disk BASIC BIN Trailer */
}



int create_os9_module(filename)
char *filename;
{
	FILE *ofp;


	static unsigned int base_addr = 0;
	unsigned int moduleSize = 0;	/* OS-9 module header size */
	int headerSize;
	int dataSize;
	int nameOffset, execOffset;
	unsigned int addr;
	char *modName;
	char headerParity;
	char typelang, attrev;
	char edition;
	int acc;


	/* Initialize variables */
	dataSize = 0;		/* Data area size */
	headerParity = 0;
	_crc[0] = 0xFF;		/* CRC */
	_crc[1] = 0xFF;
	_crc[2] = 0xFF;
	moduleSize = 0;
	modName = "test";
	edition = 1;

	/* Create the file that the module will reside in */

	ofp = fopen(filename, "w+");
	if (ofp == NULL)
	{
		return 1;
	}


	/* Start Generating Module */
	/* Module signature */
	fputc(0x87, ofp);
	fputc(0xCD, ofp);
	compute_crc(0x87); compute_crc(0xCD);
	headerParity ^= 0x87;
	headerParity ^= 0xCD;

	/* now that we know module name, compute headerSize */
	headerSize = 0x0D + strlen(modName) + sizeof(edition);

	/* find all _CODE areas and add up sizes */

	moduleSize += headerSize + 3;

	fputc(moduleSize >> 8, ofp);
	fputc(moduleSize & 0xFF, ofp);
	compute_crc(moduleSize >> 8); compute_crc(moduleSize & 0xFF);
	headerParity ^= moduleSize >> 8;
	headerParity ^= moduleSize & 0xFF;
		
	/* Write module name offset (assumed for now) */
	nameOffset = 0x0D;
	fputc(nameOffset >> 8, ofp);
	fputc(nameOffset & 0xFF, ofp);
	compute_crc(nameOffset >> 8); compute_crc(nameOffset & 0xFF);
	headerParity ^= nameOffset >> 8;
	headerParity ^= nameOffset & 0xFF;
		
	/* module type/lang (assume prgrm+objct for now) */
/*	typelang = mainlinep->m_typelang; */
	fputc(typelang, ofp);
	compute_crc(typelang);
	headerParity ^= typelang;
		
	/* module attr/rev (assume reent+0 for now) */
/*	attrev = mainlinep->m_attrev; */
	fputc(attrev, ofp);
	compute_crc(attrev);
	headerParity ^= attrev;
		
	/* header check (computed at end) */
	headerParity = ~headerParity;
	fputc(headerParity, ofp);
	compute_crc(headerParity);
		
	/* execution offset */
/*	execOffset = (int)mainlinep->m_modent + headerSize; */
	fputc(execOffset >> 8, ofp);
	fputc(execOffset & 0xFF, ofp);
	compute_crc(execOffset >> 8); compute_crc(execOffset & 0xFF);
		
	/* Compute data size - tally _BSS and _DATA areas */
	fputc(dataSize >> 8, ofp);
	fputc(dataSize & 0xFF, ofp);
	compute_crc(dataSize >> 8); compute_crc(dataSize & 0xFF);
	/* module name */
	for (acc = 0; acc < strlen(modName) - 1; acc++)
	{
		fputc(modName[acc], ofp);
		compute_crc(modName[acc]);
	}
	fputc(modName[acc] | 0x80, ofp);
	compute_crc(modName[acc] | 0x80);
		
	/* edition */
/*	edition = mainlinep->m_edition; */
	fputc(edition, ofp);
	compute_crc(edition);

#if 0
	switch (i) {
		case 1:  /* data */
			/* calculate chunk address */
			addr = (rtval[0]<<8) + rtval[1];

#if 0
			/* fill gap */
			while (base_addr < addr) {
				fputc(0, ofp);
				base_addr++;
			}
#endif

			for (i = 2; i < rtcnt ; i++) {
				if (rtflg[i]) {
					fputc(rtval[i], ofp);
					compute_crc(rtval[i]);
					base_addr++;
				}
			}

			break;

		case 0:  /* final hunk */
			// Compute and write the CRC
			fputc(~_crc[0], ofp);
			fputc(~_crc[1], ofp);
			fputc(~_crc[2], ofp);
			break;
	}
#endif
}


static int compute_crc(a)
unsigned char a;
{
	a ^= _crc[0];
	_crc[0] = _crc[1];
	_crc[1] = _crc[2];
	_crc[1] ^= (a >> 7);
	_crc[2] = (a << 1);
	_crc[1] ^= (a >> 2);
	_crc[2] ^= (a << 6);
	a ^= (a << 1);
	a ^= (a << 2);
	a ^= (a << 4);
	if (a & 0x80)
	{
		_crc[0] ^= 0x80;
		_crc[2] ^= 0x21;
	}

	return;
}


