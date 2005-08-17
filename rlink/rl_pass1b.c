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

extern unsigned t_code,
                t_idat,
                t_udat,
                t_idpd,
                t_udpd,
                t_stac,
                t_dt,
                t_dd;
extern int      dup_fnd;

int             pass1b(ob_start, lfiles, lfile_count)
	struct ob_files **ob_start;
	char           *lfiles[];
	int             lfile_count;
{
	struct ob_files *ob_cur;
	int             i;

	DBGPNT(("Starting library files\n"));

	/* Process library files to resolve remaining undefined symbols */

	ob_cur = *ob_start;
	while (ob_cur->next != NULL)
		ob_cur = ob_cur->next;

	for (i = 0; i < lfile_count; i++)
	{
		FILE           *fp;
		unsigned        modcount;

		modcount = 0;

		fp = fopen(lfiles[i], "r");

		DBGPNT(("Doing file: %s\n", lfiles[i]));

		if (fp == NULL)
		{
			fprintf(stderr, "linker fatal: library file %s could not be opened\n", lfiles[i]);
			return 1;
		}

		do
		{
			unsigned        count,
			                addrof;
			struct ob_files *ob_temp;

			modcount++;
			addrof = 0;
			ob_temp = malloc(sizeof(struct ob_files));

			if (ob_temp == NULL)
			{
				fprintf(stderr, "linker fatal: out of memory\n");
				return 1;
			}

			ob_temp->next = NULL;
			ob_temp->filename = lfiles[i];
			ob_temp->fp = fp;
			ob_temp->symbols = NULL;
			ob_temp->exts = NULL;

			if (fread(&(ob_temp->hd), sizeof(binhead), 1, fp) == 0)
			{
				break;	/* All done */
			}

			if (ob_temp->hd.h_sync != ROFSYNC)
			{
				if (modcount == 1)
				{
					fprintf(stderr, "linker fatal: '%s' is not a library file\n", lfiles[i]);
					return 1;
				}
				else
				{
					fprintf(stderr, "linker fatal: module number %d in library %s is corrupt\n", modcount, lfiles[i]);
					return 1;
				}
			}

			getname(ob_temp->modname, fp);

			DBGPNT(("Doing file: %s, mod %s\n", lfiles[i], ob_temp->modname));

			if (ob_temp->hd.h_tylan != 0)
			{
				fprintf(stderr, "linker fatal: library file %s, module %s must be type zero\n", lfiles[i], ob_temp->modname);
				return 1;
			}

			if (ob_temp->hd.h_valid)
			{
				fprintf(stderr, "linker fatal: library file: %s, module: '%s' contains assembly errors\n", lfiles[i], ob_temp->modname);
				return 1;
			}

			count = getwrd(fp);

			while (count--)
			{
				struct exp_sym *es_temp;
				int             used;

				es_temp = malloc(sizeof(struct exp_sym));

				if (es_temp == NULL)
				{
					fprintf(stderr, "linker fatal: out of memory\n");
					return 1;
				}

				getname(es_temp->name, fp);
				es_temp->flag = getc(fp);
				es_temp->offset = getwrd(fp);
				es_temp->next = NULL;

				/*
				 * Now find and remove this symbol from the
				 * external references table
				 */
				used = rm_exref(es_temp->name, *ob_start);

				/*
				 * If symbol is a unused constant, don't add
				 * it
				 */

				if (es_temp->flag & CODENT)
				{
					if (((es_temp->flag & CONENT) || (es_temp->flag & SETENT)))
					{
						if (used == 0)
						{
							free(es_temp);
							es_temp = NULL;
						}
					}
				}

				if (es_temp != NULL)
				{
					es_temp->next = ob_temp->symbols;
					ob_temp->symbols = es_temp;
					addrof += used;
				}

			}

			/*
			 * Record the position of the start of the object
			 * code
			 */
			ob_temp->object = ftell(fp);

			/* Grind past the object code and initialized data */
			fseek(fp, ob_temp->hd.h_ocode + ob_temp->hd.h_ddata + ob_temp->hd.h_data, 1);

			if (addrof > 0)
			{

				/*
				 * Library module is needed, add it to the
				 * list
				 */

				/*
				 * Now add external references to linked list
				 * - only if they not already in the global
				 * list
				 */

				count = getwrd(fp);

				while (count--)
				{
					struct ext_ref *er_temp;

					er_temp = malloc(sizeof(struct ext_ref));
					getname(er_temp->name, fp);
					er_temp->next = NULL;
					fseek(fp, getwrd(fp) * 3, 1);

					/* Check if name is listed in globals */
					if (check_name(*ob_start, er_temp->name))
					{

						/*
						 * Add name to unknown
						 * symbols
						 */
						er_temp->next = ob_temp->exts;
						ob_temp->exts = er_temp;
					}
					else
					{
						free(er_temp);
					}
				}

				ob_cur->next = ob_temp;

				t_code += ob_temp->hd.h_ocode;
				t_idat += ob_temp->hd.h_data;
				t_udat += ob_temp->hd.h_glbl;
				t_idpd += ob_temp->hd.h_ddata;
				t_udpd += ob_temp->hd.h_dglbl;
				t_stac += ob_temp->hd.h_stack;

				ob_cur = ob_temp;

				DBGPNT(("\nNeeded %s, mod %s\n", ob_temp->filename, ob_temp->modname));
				/* dmp_ext( *ob_start ); */

				/*
				 * count up local references that need
				 * data-data or data-text adjustments
				 */
				ob_temp->locref = ftell(ob_temp->fp);
				count = getwrd(fp);
				while (count--)
				{
					unsigned        flag;
					unsigned        offset;

					flag = getc(fp);
					offset = getwrd(fp);

					if (flag & CODLOC)
					{
					}
					else
					{

						DBGPNT(("mod: %s (%d) (.l) ", ob_temp->modname, count));
						ftext(flag, DEF | REF);
						DBGPNT(("\n"));

						if (flag & CODENT)
							t_dt++;
						else
							t_dd++;
					}
				}
			}
			else
			{
				/* ROF was not used, release it */

				DBGPNT(("\n%s of %s was not needed\n\n", ob_temp->modname, ob_temp->filename));

				while (ob_temp->symbols != NULL)
				{
					struct exp_sym *next;

					next = ob_temp->symbols->next;
					free(ob_temp->symbols);
					ob_temp->symbols = next;
				}

				free(ob_temp);

				/* Grind past external references */
				count = getwrd(fp);
				while (count--)
				{
					char            tmpname[SYMLEN + 1];

					getname(tmpname, fp);
					fseek(fp, getwrd(fp) * 3, 1);
				}

				/* Spin past local references */
				count = getwrd(fp);
				fseek(fp, count * 3, SEEK_CUR);

			}


			/*
			 * Added to get around double zeros at the end of
			 * some ROF files -- BGP
			 */
			{
				char            c = getc(fp);

				if (c == 0)
				{
					getc(fp);
				}
				else
				{
					ungetc(c, fp);
				}
			}


		} while (!feof(fp));

		clearerr(fp);
	}

	return 0;
}
