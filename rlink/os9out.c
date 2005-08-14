#include <stdio.h>
#include <string.h>
#include "rlink.h"


static int      compute_crc();

unsigned char   _crc[3];
FILE           *ofp;


int             os9_header(obh, filename)
	struct object_header *obh;
	char           *filename;
{
	char            headerParity;
	int             acc;

	/* Initialize variables */
	headerParity = 0;
	_crc[0] = 0xFF;		/* CRC */
	_crc[1] = 0xFF;
	_crc[2] = 0xFF;

	/* Adjust module size for OS-9 extras */
	obh->module_size += 14	/* module header */
			+ 3;	/* CRC bytes */
	obh->execution_offset += 14;	/* add module header */

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
	compute_crc(0x87);
	compute_crc(0xCD);
	headerParity ^= 0x87;
	headerParity ^= 0xCD;

	fputc(obh->module_size >> 8, ofp);
	fputc(obh->module_size & 0xFF, ofp);
	compute_crc(obh->module_size >> 8);
	compute_crc(obh->module_size & 0xFF);
	headerParity ^= obh->module_size >> 8;
	headerParity ^= obh->module_size & 0xFF;

	/* Write module name offset (assumed for now) */
	fputc(obh->offset_to_module_name >> 8, ofp);
	fputc(obh->offset_to_module_name & 0xFF, ofp);
	compute_crc(obh->offset_to_module_name >> 8);
	compute_crc(obh->offset_to_module_name & 0xFF);
	headerParity ^= obh->offset_to_module_name >> 8;
	headerParity ^= obh->offset_to_module_name & 0xFF;

	/* module type/lang (assume prgrm+objct for now) */
	fputc(obh->type_language, ofp);
	compute_crc(obh->type_language);
	headerParity ^= obh->type_language;

	/* module attr/rev (assume reent+0 for now) */
	fputc(obh->attr_rev, ofp);
	compute_crc(obh->attr_rev);
	headerParity ^= obh->attr_rev;

	/* header check (computed at end) */
	headerParity = ~headerParity;
	fputc(headerParity, ofp);
	compute_crc(headerParity);

	/* execution offset */
	fputc(obh->execution_offset >> 8, ofp);
	fputc(obh->execution_offset & 0xFF, ofp);
	compute_crc(obh->execution_offset >> 8);
	compute_crc(obh->execution_offset & 0xFF);

	/* Compute data size - tally _BSS and _DATA areas */
	fputc(obh->permanent_storage_size >> 8, ofp);
	fputc(obh->permanent_storage_size & 0xFF, ofp);
	compute_crc(obh->permanent_storage_size >> 8);
	compute_crc(obh->permanent_storage_size & 0xFF);

	/* module name */
	for (acc = 0; acc < strlen(obh->module_name) - 1; acc++)
	{
		fputc(obh->module_name[acc], ofp);
		compute_crc(obh->module_name[acc]);
	}
	fputc(obh->module_name[acc] | 0x80, ofp);
	compute_crc(obh->module_name[acc] | 0x80);

	/* edition */
	fputc(obh->edition, ofp);
	compute_crc(obh->edition);

	return 0;
}

int             os9_body(obh, data, size)
	struct object_header *obh;
	char           *data;
	size_t          size;
{
	int             i;

	for (i = 0; i < size; i++)
	{
		fputc(data[i], ofp);
		compute_crc(data[i]);
	}

	return 0;
}

int             os9_body_byte(obh, byte)
	struct object_header *obh;
	int             byte;
{
	fputc(byte, ofp);
	compute_crc(byte);

	return 0;
}

int             os9_tail(obh)
	struct object_header *obh;
{
	fputc(~_crc[0], ofp);
	fputc(~_crc[1], ofp);
	fputc(~_crc[2], ofp);

	fclose(ofp);

	return 0;
}

static int      compute_crc(a)
	unsigned char   a;
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

	return 0;
}
