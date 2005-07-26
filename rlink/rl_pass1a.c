/**********************************************************************
 *
 * RLINK - Relocatable Linker - Pass 1a
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

extern unsigned t_code, t_idat, t_udat, t_idpd, t_udpd, t_stac, t_dt, t_dd;	
extern int dup_fnd, dupfnd;

int pass1a( ob_start, rfiles, rfile_count, B09EntPt )
struct ob_files **ob_start;
char *rfiles[];
int rfile_count;
char *B09EntPt;
{
	int	i;
	struct ob_files *ob_cur;

	for (i = 0; i < rfile_count; i++)
	{
		struct ob_files *ob_temp;
		unsigned count;
		
		ob_temp = malloc( sizeof( struct ob_files  ));
		
		if( ob_temp == NULL )
		{
			fprintf( stderr, "linker fatal: Out of memory\n" );
			return 1;
		}
		
		if( i == 0 )
		{
			*ob_start = ob_temp;
			ob_cur = *ob_start;
		}
		else
		{
			ob_cur->next = ob_temp;
			ob_cur = ob_temp;
		}
		
		ob_cur->next = NULL;
		ob_cur->filename = rfiles[i];
		ob_cur->symbols = NULL;
		ob_cur->exts = NULL;
		ob_cur->fp = fopen(rfiles[i], "r");

		if (ob_cur->fp != NULL)
			fread(&(ob_cur->hd), sizeof(binhead), 1, ob_cur->fp);
		else
		{
			fprintf(stderr, "linker fatal: cannot open file %s\n", rfiles[i]);
			return 1;
		}
	
		/* Twiddle bytes if necessary */
		
		ob_cur->hd.h_tylan = ntohs(ob_cur->hd.h_tylan);
		ob_cur->hd.h_glbl = ntohs(ob_cur->hd.h_glbl);
		ob_cur->hd.h_dglbl = ntohs(ob_cur->hd.h_dglbl);
		ob_cur->hd.h_data = ntohs(ob_cur->hd.h_data);
		ob_cur->hd.h_ddata = ntohs(ob_cur->hd.h_ddata);
		ob_cur->hd.h_ocode = ntohs(ob_cur->hd.h_ocode);
		ob_cur->hd.h_stack = ntohs(ob_cur->hd.h_stack);
		ob_cur->hd.h_entry = ntohs(ob_cur->hd.h_entry);
		
		/* Accumulate totals */
		t_code += ob_cur->hd.h_ocode;
		t_idat += ob_cur->hd.h_data;
		t_udat += ob_cur->hd.h_glbl;
		t_idpd += ob_cur->hd.h_ddata;
		t_udpd += ob_cur->hd.h_dglbl;
		t_stac += ob_cur->hd.h_stack;
		
		/* Check for validity of ROF file */
		
		if( ob_cur->hd.h_sync != ROFSYNC )
		{
			fprintf( stderr, "linker fatal: '%s' is not a relocatable module\n", rfiles[i] );
			return 1;
		}
		
		if( B09EntPt == NULL )
		{
			if( i==0 ) /* First ROF file needs special header */
			{
				if( ob_cur->hd.h_tylan == 0 )
				{
					fprintf( stderr, "linker fatal: '%s' contains no mainline\n", rfiles[i] );
					return 1;
				}
			}
			else
			{
				if( ob_cur->hd.h_tylan != 0 )
				{
					fprintf( stderr, "linker fatal: mainline found in both %s and %s\n", (*ob_start)->filename, rfiles[i] );
					return 1;
				}
			}
		}
		
		if( ob_cur->hd.h_valid )
		{
			fprintf( stderr, "linker fatal: ROF file: %s must contain valid object code\n", rfiles[i] );
			return 1;
		}
		
		/* Get module name */
		getname( ob_cur->modname, ob_cur->fp ); 

		/* Add external symbols to linked list */
		
	     count=getwrd( ob_cur->fp );
		
	     while( count-- )
	     {
	     	struct exp_sym *es_temp = malloc( sizeof( struct exp_sym) );
	     	
	     	if( es_temp == NULL )
	     	{
				fprintf( stderr, "linker fatal: Out of memory\n" );
				return 1;
	     	}

	     	getname( es_temp->name, ob_cur->fp );
	     	es_temp->flag = getc(ob_cur->fp);
	     	es_temp->offset = getwrd(ob_cur->fp );

			dup_fnd += chk_dup( es_temp, *ob_start, ob_cur->modname );

			es_temp->next = ob_cur->symbols;
			ob_cur->symbols = es_temp;
			
			/* Now find and remove this symbol from the external references table */
			rm_exref( es_temp->name, *ob_start );

	     }
	     
		/* Record the position of the start of the object code */
		ob_cur->object = ftell( ob_cur->fp );
	     
		/* Grind past the object code and initialized data */

		fseek( ob_cur->fp, ob_cur->hd.h_ocode + ob_cur->hd.h_ddata + ob_cur->hd.h_data, 1 );
		
		/* Now add external references to linked list - only if they not already in the global list */
		
		count = getwrd( ob_cur->fp );
		
		while( count-- )
		{
			struct ext_ref *er_temp;
			
			er_temp = malloc( sizeof( struct ext_ref ) );
			getname( er_temp->name, ob_cur->fp );
			fseek( ob_cur->fp, getwrd(ob_cur->fp) * 3, 1 );
			
			/* Check if name is listed in globals */
			if( check_name( *ob_start, er_temp->name ) )
			{
				/* Add name to unknown symbols */
				er_temp->next = ob_cur->exts;
				ob_cur->exts = er_temp;
			}
			else
			{
				free( er_temp );
			}
		}

		/* count up local references that need data-data or data-text adjustments */
		ob_cur->locref = ftell( ob_cur->fp );
		
		count = getwrd( ob_cur->fp );
		while( count-- )
		{
			unsigned flag;
			unsigned offset;
			
			flag = getc( ob_cur->fp );
			offset = getwrd( ob_cur->fp );
			
			if( flag & CODLOC )
			{}
			else
			{
				DBGPNT(( "mod: %s (%d) (.r) ",ob_cur->modname, count ));
 				ftext(flag, DEF|REF);
				DBGPNT(("\n" ));
				
				if( flag & CODENT )
					t_dt++;
				else
					t_dd++;
			}
		}
	}

	return 0;
}


