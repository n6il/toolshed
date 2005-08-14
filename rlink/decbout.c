#include <stdio.h>
#include <string.h>
#include "rlink.h"


FILE           *ofp;


int             decb_header(obh, filename)
	struct object_header *obh;
	char           *filename;
{
	/* Create the file that the module will reside in */

	ofp = fopen(filename, "w+");
	if (ofp == NULL)
	{
		return 1;
	}

	fputc('\x00', ofp);	/* Header flag */
	fputc(obh->decb.segment_size >> 8, ofp);	/* Segment size hi */
	fputc(obh->decb.segment_size & 0xFF, ofp);	/* Segment size lo */
	fputc(obh->decb.org_offset >> 8, ofp);	/* Org address hi */
	fputc(obh->decb.org_offset & 0xFF, ofp);	/* Org address lo */

	return 0;
}

int             decb_body(obh, data, size)
	struct object_header *obh;
	char           *data;
	size_t          size;
{
	int             i;

	for (i = 0; i < size; i++)
	{
		fputc(data[i], ofp);
	}

	return 0;
}

int             decb_body_byte(obh, byte)
	struct object_header *obh;
	int             byte;
{
	fputc(byte, ofp);

	return 0;
}

int             decb_tail(obh)
	struct object_header *obh;
{
	fputc('\xFF', ofp);	/* Trailer flag */
	fputc('\x00', ofp);	/* Segment size hi (0) */
	fputc('\x00', ofp);	/* Segment size lo (0) */

	fclose(ofp);
	
	return 0;
}