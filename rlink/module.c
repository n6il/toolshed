#include <stdio.h>
#include <string.h>
#include "rlink.h"
#include "module.h"

static int      compute_crc();

unsigned char   _crc[3];
FILE           *ofp;


int             create_decb_module()
{
	/* Write Disk BASIC BIN Header */

	/* Write Disk BASIC BIN Code/Data Segment */

	/* Write Disk BASIC BIN Trailer */

	return 0;
}

int             XXX_header(obh, filename)
	struct object_header *obh;
	char           *filename;
{
	char            headerParity;
	int             acc;

	if (obh->kind != object_kind_os9)
	{
		return 1;
	}

	/* Initialize variables */
	headerParity = 0;
	_crc[0] = 0xFF;		/* CRC */
	_crc[1] = 0xFF;
	_crc[2] = 0xFF;

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

	fputc(obh->os9.module_size >> 8, ofp);
	fputc(obh->os9.module_size & 0xFF, ofp);
	compute_crc(obh->os9.module_size >> 8);
	compute_crc(obh->os9.module_size & 0xFF);
	headerParity ^= obh->os9.module_size >> 8;
	headerParity ^= obh->os9.module_size & 0xFF;

	/* Write module name offset (assumed for now) */
	fputc(obh->os9.offset_to_module_name >> 8, ofp);
	fputc(obh->os9.offset_to_module_name & 0xFF, ofp);
	compute_crc(obh->os9.offset_to_module_name >> 8);
	compute_crc(obh->os9.offset_to_module_name & 0xFF);
	headerParity ^= obh->os9.offset_to_module_name >> 8;
	headerParity ^= obh->os9.offset_to_module_name & 0xFF;

	/* module type/lang (assume prgrm+objct for now) */
	fputc(obh->os9.type_language, ofp);
	compute_crc(obh->os9.type_language);
	headerParity ^= obh->os9.type_language;

	/* module attr/rev (assume reent+0 for now) */
	fputc(obh->os9.attr_rev, ofp);
	compute_crc(obh->os9.attr_rev);
	headerParity ^= obh->os9.attr_rev;

	/* header check (computed at end) */
	headerParity = ~headerParity;
	fputc(headerParity, ofp);
	compute_crc(headerParity);

	/* execution offset */
	fputc(obh->os9.execuation_offset >> 8, ofp);
	fputc(obh->os9.execuation_offset & 0xFF, ofp);
	compute_crc(obh->os9.execuation_offset >> 8);
	compute_crc(obh->os9.execuation_offset & 0xFF);

	/* Compute data size - tally _BSS and _DATA areas */
	fputc(obh->os9.permanent_storage_size >> 8, ofp);
	fputc(obh->os9.permanent_storage_size & 0xFF, ofp);
	compute_crc(obh->os9.permanent_storage_size >> 8);
	compute_crc(obh->os9.permanent_storage_size & 0xFF);

	/* module name */
	for (acc = 0; acc < strlen(obh->os9.module_name) - 1; acc++)
	{
		fputc(obh->os9.module_name[acc], ofp);
		compute_crc(obh->os9.module_name[acc]);
	}
	fputc(obh->os9.module_name[acc] | 0x80, ofp);
	compute_crc(obh->os9.module_name[acc] | 0x80);

	/* edition */
	fputc(obh->os9.edition, ofp);
	compute_crc(obh->os9.edition);

	return 0;
}

int             XXX_body(obh, data, size)
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

int             XXX_body_byte(obh, byte)
	struct object_header *obh;
	int             byte;
{
	fputc(byte, ofp);
	compute_crc(byte);

	return 0;
}

int             XXX_tail(obh)
	struct object_header *obh;
{
	if (obh->kind == object_kind_os9)
	{
		fputc(~_crc[0], ofp);
		fputc(~_crc[1], ofp);
		fputc(~_crc[2], ofp);
	}
	else if (obh->kind == object_kind_rsdos);
	{
	}

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
