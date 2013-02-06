/**********************************************************************
 *
 * RLINK - Relocatable Linker
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
#if defined(UNIX) || defined(__APPLE__)
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#endif
#include "rlink.h"

int             link();
char           *flagtext();
unsigned        adjoff();
int             help();
int             asign_sm();
int             dmp_ext();

extern int      pass1a();
extern int      pass1b();
extern int      pass2();

unsigned        t_code,
                t_idat,
                t_udat,
                t_idpd,
                t_udpd,
                t_stac,
                t_dt,
                t_dd;
int             dup_fnd;

/* The 'main' function */
int             main(argc, argv)
	int             argc;
	char          **argv;
{
	char           *rfiles[MAX_RFILES];
	char           *lfiles[MAX_LFILES];
	char           *ofile,
	               *modname;
	char           *B09EntPt;
	int             edition,
	                extramem,
	                okstatic,
	                omitC;
	int             printmap,
	                printsym;
	int             rfile_count,
	                lfile_count,
	                i;

	fprintf(stderr, "RLINK IS DEPRECATED! USE LWTOOLS INSTEAD!!!\n");

	modname = NULL;
	ofile = NULL;
	B09EntPt = NULL;
	edition = -1;
	extramem = 0;
	printmap = 0;
	printsym = 0;
	okstatic = 0;
	omitC = 0;

	rfile_count = 0;
	lfile_count = 0;

	/* Check sctrucure alignment */
	if (sizeof(binhead) != 28)
	{
		fprintf(stderr, "compiler error: Relocatable object header def incorrectly sized\n");
		return 1;
	}


	/* Assume default output is OS-9 module */
	XXX_header = os9_header;
	XXX_body_byte = os9_body_byte;
	XXX_body = os9_body;
	XXX_tail = os9_tail;

	/* Parse options */
	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			/* option -- process it */
			switch (argv[i][1])
			{
			case 'e':
			case 'E':
				/* Edition */
				{
					char           *p = &argv[i][2];

					if (argv[i][2] == '=')
					{
						p++;
					}

					edition = atoi(p);
				}
				break;

			case 'B':
				/* Disk BASIC Modle */
				{
					XXX_header = decb_header;
					XXX_body_byte = decb_body_byte;
					XXX_body = decb_body;
					XXX_tail = decb_tail;
				}
				break;

			case 'M':
				/* Extra Memory */
				{
					char           *p = &argv[i][2];

					if (argv[i][2] == '=')
					{
						p++;
					}

					extramem = atoi(p);

					if (p[strlen(p) - 1] == 'K' || p[strlen(p) - 1] == 'k')
						extramem *= 1024;
					else
						extramem *= 256;

				}
				break;

			case 'm':
				/* Print linkage map */
				{
					printmap = 1;
				}
				break;

			case 's':
				/* Print symbol table */
				{
					printsym = 1;
				}
				break;

			case 'n':
				/* Name */
				{
					char           *p = &argv[i][2];

					if (argv[i][2] == '=')
					{
						p++;
					}

					modname = p;
				}
				break;

			case 'o':
				/* Output object file */
				{
					char           *p = &argv[i][2];

					if (argv[i][2] == '=')
					{
						p++;
					}

					if (ofile == NULL)
					{
						ofile = p;
						if (modname == NULL)
						{

							/*
							 * Set the module
							 * name
							 */
							modname = (char *)basename(p);
						}
					}
					else
					{
						fprintf(stderr, "linker fatal: -o option already specified\n");
						exit(1);
					}
				}
				break;

			case 'l':
				/* Library */
				{
					char           *p = &argv[i][2];

					if (argv[i][2] == '=')
						p++;

					lfiles[lfile_count] = p;
					lfile_count++;
					if (lfile_count > MAX_LFILES)
					{
						fprintf(stderr, "linker fatal: too many library files\n");
						exit(1);
					}
				}
				break;

			case 'b':
				/* BASIC09 Entry point */
				{
					char           *p = &argv[i][2];

					if (argv[i][2] == '=')
						p++;

					if (B09EntPt == NULL)
					{
						B09EntPt = p;
					}
					else
					{
						fprintf(stderr, "linker fatal: -b option already specified\n");
						exit(1);
					}
				}
				break;

			case 't':
				/* Allow static data in BASIC09 modules */
				{
					okstatic = 1;
				}
				break;

			case 'y':
				/* omit C related data */
				{
					omitC = 1;
				}
				break;

			case '?':
				help();
				exit(0);

			default:
				fprintf(stderr, "linker fatal: unknown option %c in %s\n", argv[i][1], argv[i]);
				exit(1);
			}
		}
		else
		{
			/* file -- add it to the list */
			rfiles[rfile_count] = argv[i];
			rfile_count++;
			if (rfile_count > MAX_RFILES)
			{
				fprintf(stderr, "linker fatal: too many ROF files\n");
				exit(1);
			}
		}
	}

	if (ofile == NULL)
	{
		fprintf(stderr, "linker fatal: no output file\n");
		exit(1);
	}

	/* Call the function which does all the work! */

	return link(rfiles, rfile_count, lfiles, lfile_count, ofile, modname, edition, extramem, B09EntPt, printmap, printsym, okstatic, omitC);
}



int             help()
{
	fprintf(stderr, "Usage: rlink <opts> source_file [source_file ...] <opts>\n");
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "   -o[=]file      output object file\n");
	fprintf(stderr, "   -n[=]name      module name of the object file\n");
	fprintf(stderr, "   -l[=]path      additional library\n");
	fprintf(stderr, "   -E[=]edition   edition number in the module\n");
	fprintf(stderr, "   -M[=]size[K]   additional number of pages [or kilobytes] of memory\n");
	fprintf(stderr, "   -m             print the linkage map\n");
	fprintf(stderr, "   -s             print the symbol table\n");
	fprintf(stderr, "   -b=ept         make callable from BASIC09\n");
	fprintf(stderr, "   -B             emit Disk BASIC loadable module\n");
	fprintf(stderr, "   -t             allow static data to appear in BASIC09 module\n");
	fprintf(stderr, "   -y             omit C related data\n");

	return 0;
}



int             link(rfiles, rfile_count, lfiles, lfile_count, ofile, modname, edition, extramem, B09EntPt, printmap, printsym, okstatic, omitC)
	char           *rfiles[];
	int             rfile_count;
	char           *lfiles[];
	int             lfile_count;
	char           *ofile;
	char           *modname,
	               *B09EntPt;
	int             edition,
	                extramem,
	                printmap,
	                printsym,
	                okstatic,
	                omitC;
{
	struct ob_files *ob_start;
	struct ob_files *ob_cur;
	struct exp_sym *es_cur;
	int             once = 0;

	int             result;

	/*
	 * We have the ROF input files, the library input files and the
	 * output file. Now let's go to work and link 'em!
	 */
	if (rfile_count == 0)
	{
		fprintf(stderr, "linker fatal: no files specified to link\n");
		return 1;
	}

	t_code = t_idat = t_udat = t_idpd = t_udpd = t_stac = t_dt = t_dd = 0;

	result = pass1a(&ob_start, rfiles, rfile_count, B09EntPt);

	if (result != 0)
		return result;

	result = pass1b(&ob_start, lfiles, lfile_count);

	if (result != 0)
		return result;

	/* Check if duplicate globals are found */
	if (dup_fnd > 0)
	{
		fprintf(stderr, "linker fatal: name clash\n");
		return 1;
	}

	/* Check if direct page usage overflows */

	if (t_idpd + t_udpd > 0xff)
	{
		fprintf(stderr, "linker fatal: direct page allocation is %d bytes\n", t_idpd + t_udpd);
		return 1;
	}

	/* Adjust data offsets */
	ob_cur = ob_start;

	ob_cur->Code = 14 + strlen(modname);
	ob_cur->IDpD = 0;
	ob_cur->UDpD = ob_cur->IDpD + t_idpd;
	ob_cur->IDat = ob_cur->UDpD + t_udpd;
	ob_cur->UDat = ob_cur->IDat + t_idat;

	if (ob_cur->next != NULL)
	{
		struct ob_files *prev;

		prev = ob_cur;
		ob_cur = ob_cur->next;
		do
		{
			ob_cur->Code = prev->Code + prev->hd.h_ocode;
			ob_cur->IDpD = prev->IDpD + prev->hd.h_ddata;
			ob_cur->UDpD = prev->UDpD + prev->hd.h_dglbl;
			ob_cur->IDat = prev->IDat + prev->hd.h_data;
			ob_cur->UDat = prev->UDat + prev->hd.h_glbl;
			prev = ob_cur;
		} while ((ob_cur = ob_cur->next) != NULL);
	}

	/* Adjust exported symbols */

	ob_cur = ob_start;

	while (ob_cur != NULL)
	{
		es_cur = ob_cur->symbols;

		while (es_cur != NULL)
		{
			es_cur->offset = adjoff(es_cur->offset, es_cur->flag, ob_cur);
			es_cur = es_cur->next;
		}
		ob_cur = ob_cur->next;
	}

	/* Add special symbols (linker generated symbols) */

	if (omitC == 0)
	{
		/* Fix for -o=blahblahblah/file which would throw off
		 * the strlen calculation - BGP 04/06/07
		 */
		char *oofile = strrchr(ofile, '/');

		if (oofile != NULL)
		{
			oofile++;
		}
		else
		{
			oofile = ofile;
		}

		if (asign_sm(ob_start, "etext", CODENT, 14 + strlen(oofile) + t_code) != 0)
			return 1;
		if (asign_sm(ob_start, "btext", CODENT, 0) != 0)
			return 1;
		if (asign_sm(ob_start, "edata", INIENT, t_idpd + t_udpd + t_idat) != 0)
			return 1;
		if (asign_sm(ob_start, "end", 0, t_idpd + t_udpd + t_idat + t_udat) != 0)
			return 1;
		if (asign_sm(ob_start, "dpsiz", DIRENT, t_idpd + t_udpd) != 0)
			return 1;
	}

	/* Check if there are any unresolved symbols */
	ob_cur = ob_start;
	do
	{
		struct ext_ref *ex_cur;

		if (ob_cur->exts != NULL)
		{
			if (once == 0)
			{
				fprintf(stderr, "Unresolved references:\n");
				once = 1;
			}

			ex_cur = ob_cur->exts;
			do
			{
				fprintf(stderr, " %-15s %-15s in %-15s\n", ex_cur->name, ob_cur->modname, ob_cur->filename);

			} while ((ex_cur = ex_cur->next));
		}
	} while ((ob_cur = ob_cur->next) != NULL);

	if (once == 1)
	{
		fprintf(stderr, "linker fatal: unresolved references\n");
		return 1;
	}

	/* Static data and BASIC09 usually dont mix */

	if (B09EntPt != NULL)
	{
		if (t_idat > 0 || t_idpd > 0)
		{
			fprintf(stderr, "no init data allowed\nlinker fatal: BASIC09 conflict\n");
			return 1;
		}

		if (t_idat > 0 || t_idpd > 0)
		{
			if (okstatic == 0)
			{
				fprintf(stderr, "no static data\nlinker fatal: BASIC09 conflict\n");
				return 1;
			}
			else
			{
				printf("BASIC09 static data size is %d byte%s.\n", t_idat + t_idpd, t_idat + t_idpd > 1 ? "s" : "");
			}
		}
	}

	/* Print link Map */
	if (printmap)
	{
		printf("Linkage map for %s  File - %s\n\n", modname, ofile);
		printf("Section          Code IDat UDat IDpD UDpD File\n\n");

		ob_cur = ob_start;

		do
		{
			printf("%-16s %4.4x %4.4x %4.4x %2.2x   %2.2x %s\n", ob_cur->modname, ob_cur->Code, ob_cur->IDat,
			       ob_cur->UDat, ob_cur->IDpD, ob_cur->UDpD, ob_cur->filename);
			es_cur = ob_cur->symbols;
			if (printsym)
			{
				while (es_cur != NULL)
				{
					printf("     %-9s %s %4.4x\n", es_cur->name, flagtext(es_cur->flag), es_cur->offset);
					es_cur = es_cur->next;
				}
			}

		} while ((ob_cur = ob_cur->next) != NULL);

		printf("                 ---- ---- ---- --\n");
		printf("                 %4.4x %4.4x %4.4x %2.2x  %2.2x\n\n", t_code, t_idat, t_udat, t_idpd, t_udpd);
	}

	DBGPNT(("Total stack space: %4.4x\n", t_stac));
	DBGPNT(("Data-Text count: %d\n", t_dt));
	DBGPNT(("Data-data count: %d\n", t_dd));

	result = pass2(&ob_start, ofile, modname, B09EntPt, extramem, edition, omitC);

	if (result != 0)
		return result;

	return 0;
}

/*
 * Assign symbol ob must be ob_start
 */

int             asign_sm(ob, name, flag, offset)
	struct ob_files *ob;
	char           *name;
	char            flag;
	unsigned        offset;
{
	struct exp_sym *exp;

	exp = malloc(sizeof(struct exp_sym));

	if (exp == NULL)
	{
		fprintf(stderr, "linker fatal: out of memory\n");
		return 1;
	}

	strcpy(exp->name, name);
	exp->flag = flag;
	exp->offset = offset;

	if (chk_dup(exp, ob, "linker") > 0)
	{
		free(exp);
		return 1;
	}

	rm_exref(name, ob);

	exp->next = ob->symbols;
	ob->symbols = exp;
	return 0;
}

int             getname(s, fp)
	register char  *s;
	FILE           *fp;
{
	while ((*s++ = getc(fp)));
	*s = '\0';
	return 0;
}

unsigned        getwrd(fp)
	FILE           *fp;
{
	unsigned        Msb,
	                Lsb;
	unsigned        nbr;

	Msb = getc(fp);
	Lsb = getc(fp);
	nbr = Msb << 8 | Lsb;
	return nbr;
}

char           *flagtext(flag)
	char            flag;
{
	if (flag & CODENT)
	{
		if ((flag & CONENT) || (flag & SETENT))
			return "cnst";
		else
			return "code";
	}
	else
	{
		if (flag & INIENT)
		{
			if (flag & DIRENT)
				return "idpd";
			else
				return "idat";
		}
		else
		{
			if (flag & DIRENT)
				return "udpd";
			else
				return "udat";
		}
	}
}

unsigned        adjoff(offset, flag, ob)
	unsigned        offset;
	char            flag;
	struct ob_files *ob;
{
	if (flag & CODENT)
	{
		if ((flag & CONENT) || (flag & SETENT))
			return offset;
		else
			return offset + ob->Code;
	}
	else
	{
		if (flag & INIENT)
		{
			if (flag & DIRENT)
				return offset + ob->IDpD;
			else
				return offset + ob->IDat;
		}
		else
		{
			if (flag & DIRENT)
				return offset + ob->UDpD;
			else
				return offset + ob->UDat;
		}
	}
}

/*
 * Check if a symbol name is in the exported symbol table Returns true, false
 */

int             check_name(ob, name)
	struct ob_files *ob;
	char           *name;
{
	do
	{
		struct exp_sym *exp;

		exp = ob->symbols;
		if (exp == NULL)
			continue;

		do
		{
			if (strcmp(name, exp->name) == 0)
			{
				return 0;
			}

		} while ((exp = exp->next));

	} while ((ob = ob->next));

	return 1;
}

/*
 * Checks if a symbol name is in the exported symbol table Prints duplicate
 * error if it is
 */

int             chk_dup(es, ob, modname)
	struct exp_sym *es;
	struct ob_files *ob;
	char           *modname;
{
	int             result = 0;

	if (ob == NULL)
		return result;

	do
	{
		struct exp_sym *ext;

		ext = ob->symbols;

		if (ext == NULL)
			continue;

		do
		{

			if (strcmp(es->name, ext->name) == 0)
			{
				fprintf(stderr, "symbol already defined: %-10s in %s and %s\n", es->name, ob->modname, modname);
				result++;
			}
		} while ((ext = ext->next));
	} while ((ob = ob->next));

	return result;
}

/* Remove external refrence from external reference table */
int             rm_exref(name, ob)
	char           *name;
	struct ob_files *ob;
{
	int             count = 0;

	while (ob != NULL)
	{
		struct ext_ref *ext,
		               *prev;

		prev = NULL;
		ext = ob->exts;
		while (ext != NULL)
		{
			if (strcmp(name, ext->name) == 0)
			{
				count++;

				if (prev == NULL)
				{
					ob->exts = ext->next;
					free(ext);
					ext = ob->exts;
				}
				else
				{
					struct ext_ref *tmp;

					tmp = ext->next;
					prev->next = tmp;
					free(ext);
					ext = tmp;
				}
			}
			else
			{
				prev = ext;
				ext = ext->next;
			}
		}

		ob = ob->next;
	}

	return count;
}

int             dmp_ext(ob)
	struct ob_files *ob;
{

	while (ob != NULL)
	{
		struct ext_ref *exts;

		DBGPNT(("   File: %s, mod: %s: ", ob->filename, ob->modname));

		exts = ob->exts;
		while (exts != NULL)
		{
			DBGPNT(("%s ", exts->name));
			exts = exts->next;
		}

		DBGPNT(("\n"));
		ob = ob->next;
	}

	return 0;
}

int             ftext(c, ref)
	char            c;
	int             ref;
{
	DBGPNT(("(%02x) ", mc(c)));

	if (ref & REF)
	{
		if (c & CODLOC)
		{
			DBGPNT(("in code"));
		}
		else
		{
			DBGPNT((c & DIRLOC ? "in dp data" : "in non-dp data"));
		}

		DBGPNT((c & LOC1BYT ? "/byte" : "/word"));

		if (c & NEGMASK)
		{
			DBGPNT(("/neg"));
		}

		if (c & RELATIVE)
		{
			DBGPNT(("/pcr"));
		}
	}

	if (ref & DEF)
	{
		if (ref & REF)
		{
			DBGPNT((" - "));
		}
		if (c & CODENT)
		{
			DBGPNT(("to code"));
		}
		else
		{
			DBGPNT((c & DIRENT ? "to dp" : "to non-dp"));
			DBGPNT((c & INIENT ? " data" : " bss"));
		}
	}

	return 0;
}
