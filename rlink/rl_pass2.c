/**********************************************************************
 *
 * RLINK - Relocatable Linker - Pass 1b
 *
 * Compatible with Microware's 'rlink' linker for the 6809.
 *
 * Written because Allen Huffman wouldn't get off his lazy
 * ass and send Tim those disks.
 *
 * Thanks for the motivation Allen!
 *
 **********************************************************************/

#include <stdio.h>
#ifdef UNIX
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#endif
#include "rlink.h"

int     (*XXX_header)();
int     (*XXX_body)();
int     (*XXX_body_byte)();
int     (*XXX_tail)();

extern unsigned t_code,
                t_idat,
                t_udat,
                t_idpd,
                t_udpd,
                t_stac,
                t_dt,
                t_dd;

static unsigned getsym();

int             pass2(ob_start, ofile, modname, B09EntPt, extramem, edition, omitC)
	struct ob_files **ob_start;
	char           *ofile;
	char           *modname;
	char           *B09EntPt;
	int             extramem;
	int             edition;
	int             omitC;
{
	struct ob_files *ob_cur;
	struct object_header obh;

	if (omitC)
	{
		obh.module_size = strlen(modname)	/* module name */
			+ t_code/* Code size of all segements */
			+ t_idpd/* Initialized direct page data of all
				 * segements */
			+ t_idat; /* Initialized data of all segments */
	}
	else
	{
		obh.module_size = strlen(modname)	/* module name */
			+ t_code/* Code size of all segements */
			+ t_idpd/* Initialized direct page data of all
				 * segements */
			+ 2	/* Linker direct page initialized data */
			+ t_idat/* Initialized data of all segments */
			+ 2	/* Linker initialized data */
			+ 2 + t_dt * 2	/* Data-text reference table */
			+ 2 + t_dd * 2	/* Data-data reference table */
			+ strlen(modname) + 1;	/* Program name (NULL
						 * terminated) */
	}

	obh.offset_to_module_name = 0x0d;

	if (B09EntPt != NULL)
		obh.type_language = 0x21;
	else
		obh.type_language = 0x11;

	obh.attr_rev = 0x81;

	if (B09EntPt == NULL)
		obh.execution_offset = (*ob_start)->hd.h_entry + strlen(modname);
	else
		obh.execution_offset = getsym(*ob_start, B09EntPt, NULL);

	/* Compute data size */
	obh.permanent_storage_size = t_stac + t_idat + t_udat + t_idpd + t_udpd + extramem;

	if( strlen( basename(modname)) > 29 )
	{
		fprintf( stderr, "linker fatal: output file name cannot exceed 29 characters\n" );
		return 1;
	}

   	(void)strncpy(obh.module_name, modname, sizeof(obh.module_name));

	obh.edition = edition;
	if (obh.edition == -1)
		obh.edition = (*ob_start)->hd.h_edit;

	if (XXX_header(&obh, ofile))
	{
		fprintf(stderr, "linker fatal: cannot open output file %s\n", ofile);
		return 1;
	}

	/* Now dump all of the code */

	ob_cur = *ob_start;

	while (ob_cur != NULL)
	{
		unsigned char  *data;
		unsigned        count;

		DBGPNT(("Object %s is %4.4lx - %4.4lx, len: %u\n", ob_cur->modname, ftell(ob_cur->fp), ftell(ob_cur->fp) + ob_cur->hd.h_ocode, ob_cur->hd.h_ocode));

		fseek(ob_cur->fp, ob_cur->object, SEEK_SET);
		data = malloc(ob_cur->hd.h_ocode);
		if (data == NULL)
		{
			fprintf(stderr, "linker fatal: out of memory\n");
			return 1;
		}

		fread(data, ob_cur->hd.h_ocode, 1, ob_cur->fp);

		/* Now patch binary */
		fseek(ob_cur->fp, ob_cur->object + ob_cur->hd.h_ocode + ob_cur->hd.h_data + ob_cur->hd.h_ddata, SEEK_SET);
		count = getwrd(ob_cur->fp);

		if (count > 0)
		{
			DBGPNT(("%u external References:\n", count));
		}

		while (count--)
		{
			char            symbol[SYMLEN + 1],
			                valueflg;
			unsigned        number,
			                value;

			getname(symbol, ob_cur->fp);
			value = getsym(*ob_start, symbol, &valueflg);
			number = getwrd(ob_cur->fp);

			DBGPNT(("%-10s %-10s %4.4x (", ob_cur->modname, symbol, value));
			ftext(valueflg, DEF);
			DBGPNT((") "));

			while (number--)
			{
				unsigned        flag;
				unsigned        offset,
				                result,
				                scratch;

				flag = getc(ob_cur->fp);
				offset = getwrd(ob_cur->fp);

				if (flag & CODLOC)
				{
					DBGPNT((" External ref patch: ("));
					ftext(flag, REF);

					if (offset > ob_cur->hd.h_ocode)
					{
						fprintf(stderr, "linker fatal: code external reference offset greater than code size\n");
						return 1;
					}

					if (flag & LOC1BYT)
						scratch = data[offset];
					else
						scratch = (unsigned) (data[offset] << 8) + data[offset + 1];

					DBGPNT((") %4.4x (%4.4x) data: %4.4x, ", offset + ob_cur->Code, offset, scratch));

					if (flag & NEGMASK)
						result = ~value;
					else
						result = value;

					switch (flag & ~(LOC1BYT | NEGMASK))
					{
					case 0x20:
						result += scratch;
						break;
					case 0xa0:
						result -= ob_cur->Code;
						result -= offset;
						result -= 2;
						break;
					default:
						fprintf(stderr, "fatal error: Unknown external reference flag %2.2x\n", flag);
						return 1;
						break;
					}

					if (flag & LOC1BYT)
						data[offset] = result;
					else
					{
						data[offset] = result >> 8;
						data[offset + 1] = result & 0xff;
					}
				}

			}

			DBGPNT(("\n"));

		}

		/* Patch local refs */

		count = getwrd(ob_cur->fp);
		while (count--)
		{
			unsigned        flag;
			unsigned        offset,
			                result;

			flag = getc(ob_cur->fp);
			offset = getwrd(ob_cur->fp);

			if (flag & CODLOC)
			{
				if (offset > ob_cur->hd.h_ocode)
				{
					fprintf(stderr, "linker fatal: code local reference offset greater than code size\n");
					return 1;
				}

				if (flag & LOC1BYT)
					result = data[offset];
				else
					result = (unsigned) (data[offset] << 8) + data[offset + 1];

				if (flag & NEGMASK)
					result = ~result;
				else
					result = result;

				if (flag & DIRENT)
				{
					if (flag & INIENT)
						result += ob_cur->IDpD;
					else
						result += ob_cur->UDpD;
				}
				else
				{
					if (flag & INIENT)
						result += ob_cur->IDat;
					else
						result += ob_cur->UDat;
				}

				if (flag & LOC1BYT)
					data[offset] = result;
				else
				{
					data[offset] = result >> 8;
					data[offset + 1] = result & 0xff;
				}

				DBGPNT((" Local ref patch ("));
				ftext(flag, DEF | REF);
				DBGPNT((") %4.4x (%4.4x)\n", offset + ob_cur->Code, offset));
			}
		}

		XXX_body(&obh, data, ob_cur->hd.h_ocode);
		free(data);

		ob_cur = ob_cur->next;
	}

	/* Now dump all of the Initialized DP data */

	ob_cur = *ob_start;

	while (ob_cur != NULL)
	{
		unsigned char  *data;
		unsigned        count;

		DBGPNT(("Initialized DP data %s is %4.4lx - %4.4lx\n", ob_cur->modname, ftell(ob_cur->fp), ftell(ob_cur->fp) + ob_cur->hd.h_ddata));

		fseek(ob_cur->fp, ob_cur->object + ob_cur->hd.h_ocode + ob_cur->hd.h_data, SEEK_SET);
		if (ob_cur->hd.h_ddata > 0)
		{
			data = malloc(ob_cur->hd.h_ddata);
			if (data == NULL)
			{
				fprintf(stderr, "linker fatal: out of memory\n");
				return 1;
			}

			fread(data, ob_cur->hd.h_ddata, 1, ob_cur->fp);
		}

		/* Adjust local references */
		fseek(ob_cur->fp, ob_cur->locref, SEEK_SET);

		count = getwrd(ob_cur->fp);
		while (count--)
		{
			unsigned        flag;
			unsigned        offset,
			                result;

			flag = getc(ob_cur->fp);
			offset = getwrd(ob_cur->fp);

			if (flag & CODLOC)
			{
			}
			else
			{
				if (flag & DIRLOC)
				{
					if (flag & LOC1BYT)
						result = data[offset];
					else
						result = (unsigned) (data[offset] << 8) + data[offset + 1];

					if (flag & NEGMASK)
						result = ~result;
					else
						result = result;

					if (flag & CODENT)
					{
						result += ob_cur->Code;
					}
					else
					{
						if (flag & DIRENT)
						{
							if (flag & INIENT)
							{
								result += ob_cur->IDpD;
							}
							else
							{
								result += ob_cur->UDpD;
							}
						}
						else
						{
							if (flag & INIENT)
							{
								result += ob_cur->IDat;
							}
							else
							{
								result += ob_cur->UDat;
							}
						}
					}

					if (flag & LOC1BYT)
						data[offset] = result;
					else
					{
						data[offset] = result >> 8;
						data[offset + 1] = result & 0xff;
					}
				}
			}
		}

		XXX_body(&obh, data, ob_cur->hd.h_ddata);
		free(data);

		/* Dump special linker initialized dp data */
		if (ob_cur == *ob_start && !omitC)
		{
			DBGPNT(("Initialized linker dp data is %4.4lx - %4.4lx\n", ftell(ob_cur->fp), ftell(ob_cur->fp) + 2));
			XXX_body_byte(&obh, t_idpd);
			XXX_body_byte(&obh, t_udpd);
		}

		ob_cur = ob_cur->next;
	}

	/* Now dump all of the Initialized data */

	ob_cur = *ob_start;

	while (ob_cur != NULL)
	{
		unsigned char  *data;
		unsigned        count;

		DBGPNT(("Initialized data %s is %4.4lx - %4.4lx\n", ob_cur->modname, ftell(ob_cur->fp), ftell(ob_cur->fp) + ob_cur->hd.h_data));
		fseek(ob_cur->fp, ob_cur->object + ob_cur->hd.h_ocode, SEEK_SET);
		if (ob_cur->hd.h_data > 0)
		{
			data = malloc(ob_cur->hd.h_data);
			if (data == NULL)
			{
				fprintf(stderr, "linker fatal: out of memory\n");
				return 1;
			}

			fread(data, ob_cur->hd.h_data, 1, ob_cur->fp);
		}

		/* Adjust local references */
		fseek(ob_cur->fp, ob_cur->locref, SEEK_SET);

		count = getwrd(ob_cur->fp);
		while (count--)
		{
			unsigned        flag;
			unsigned        offset,
			                result;

			flag = getc(ob_cur->fp);
			offset = getwrd(ob_cur->fp);

			if (flag & CODLOC)
			{
			}
			else
			{
				if (flag & DIRLOC)
				{
				}
				else
				{
					if (flag & LOC1BYT)
						result = data[offset];
					else
						result = (unsigned) (data[offset] << 8) + data[offset + 1];

					if (flag & NEGMASK)
						result = ~result;
					else
						result = result;

					if (flag & CODENT)
					{
						result += ob_cur->Code;
					}
					else
					{
						if (flag & DIRENT)
						{
							if (flag & INIENT)
							{
								result += ob_cur->IDpD;
							}
							else
							{
								result += ob_cur->UDpD;
							}
						}
						else
						{
							if (flag & INIENT)
							{
								result += ob_cur->IDat;
							}
							else
							{
								result += ob_cur->UDat;
							}
						}
					}

					if (flag & LOC1BYT)
						data[offset] = result;
					else
					{
						data[offset] = result >> 8;
						data[offset + 1] = result & 0xff;
					}
				}
			}
		}

		XXX_body(&obh, data, ob_cur->hd.h_data);
		free(data);

		/* Dump special linker initialized data */
		if (ob_cur == *ob_start && !omitC)
		{
			DBGPNT(("Initialized linker data is %4.4lx - %4.4lx\n", ftell(ob_cur->fp), ftell(ob_cur->fp) + 2));
			XXX_body_byte(&obh, (t_idat >> 8) & 0xff);
			XXX_body_byte(&obh, t_idat & 0xff);
		}

		ob_cur = ob_cur->next;
	}

	if (ob_cur && ob_cur->fp)
		DBGPNT(("Data-text table is %4.4lx - %4.4lx\n", ftell(ob_cur->fp), ftell(ob_cur->fp) + 2 + (t_dt * 2)));
	

	/* Now dump Data-text table */
	if (!omitC)
	{
		XXX_body_byte(&obh, (t_dt >> 8) & 0xff);
		XXX_body_byte(&obh, t_dt & 0xff);
	}

	ob_cur = *ob_start;
	while (ob_cur != NULL)
	{
		unsigned        count;

		fseek(ob_cur->fp, ob_cur->locref, SEEK_SET);

		count = getwrd(ob_cur->fp);
		while (count--)
		{
			unsigned        flag;
			unsigned        offset;

			flag = getc(ob_cur->fp);
			offset = getwrd(ob_cur->fp);

			if (flag & CODLOC)
			{
			}
			else
			{
				if (flag & CODENT)
				{
					if (omitC)
					{
						fprintf(stderr, "linker fatal: data-text tables not allowed in non C based modules\n");
						return 1;
					}

					if (flag & DIRLOC)
						offset += ob_cur->IDpD;
					else
						offset += ob_cur->IDat;

					XXX_body_byte(&obh, (offset >> 8) & 0xff);
					XXX_body_byte(&obh, offset & 0xff);
				}
				else
				{
				}
			}
		}

		ob_cur = ob_cur->next;
	}

	if (ob_cur && ob_cur->fp)
		DBGPNT(("Data-data table is %4.4lx - %4.4lx\n", ftell(ob_cur->fp), ftell(ob_cur->fp) + 2 + (t_dd * 2)));

	/* Now dump Data-data table */
	if (!omitC)
	{
		XXX_body_byte(&obh, (t_dd >> 8) & 0xff);
		XXX_body_byte(&obh, t_dd & 0xff);
	}

	ob_cur = *ob_start;
	while (ob_cur != NULL)
	{
		unsigned        count;

		fseek(ob_cur->fp, ob_cur->locref, SEEK_SET);

		count = getwrd(ob_cur->fp);
		while (count--)
		{
			unsigned        flag;
			unsigned        offset;

			flag = getc(ob_cur->fp);
			offset = getwrd(ob_cur->fp);

			if (flag & CODLOC)
			{
			}
			else
			{
				if (flag & CODENT)
				{
				}
				else
				{
					if (omitC)
					{
						fprintf(stderr, "linker fatal: data-data tables not allowed in non C based modules\n");
						return 1;
					}

					if (flag & DIRENT)
						offset += ob_cur->IDpD;
					else
						offset += ob_cur->IDat;

					XXX_body_byte(&obh, (offset >> 8) & 0xff);
					XXX_body_byte(&obh, offset & 0xff);
				}
			}
		}

		ob_cur = ob_cur->next;
	}

	if (!omitC)
	{
		if (ob_cur && ob_cur->fp)
			DBGPNT(("Program name is %4.4lx - %4.4lx\n", 
				ftell(ob_cur->fp), ftell(ob_cur->fp) + strlen(modname) + 1));
		/* Now dump program name as a C string */
		XXX_body(&obh, modname, strlen(modname));
		XXX_body_byte(&obh, 0);
	}

	/* Now write CRC */
	XXX_tail(&obh);

	return 0;
}

/* Get value and flag for symbol */
unsigned        getsym(ob, symbol, flag)
	struct ob_files *ob;
	char           *symbol;
	char           *flag;
{
	while (ob != NULL)
	{
		struct exp_sym *exp;

		exp = ob->symbols;

		while (exp != NULL)
		{
			if (strcmp(symbol, exp->name) == 0)
			{
				if (flag != NULL)
					*flag = exp->flag;
				return exp->offset;
			}

			exp = exp->next;
		}

		ob = ob->next;
	}

	fprintf(stderr, "linker fatal: could not find requested symbol: %s\n", symbol);
	exit(1);

	return 0;
}
