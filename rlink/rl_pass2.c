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

extern unsigned t_code, t_idat, t_udat, t_idpd, t_udpd, t_stac, t_dt, t_dd;	

static unsigned getsym();
static int compute_crc();
static int buffer_crc();

#ifdef UNIX
unsigned char _crc[3];
#else
char _crc[3];
#endif

int pass2( ob_start, ofile, modname, B09EntPt, extramem, edition, omitC )
struct ob_files **ob_start;
char *ofile;
char *modname;
char *B09EntPt;
int extramem;
int edition;
int omitC;
{
	FILE *ofp;
	struct ob_files *ob_cur;
	unsigned headerParity, moduleSize, nameOffset, execOffset, dataSize;
	unsigned typelang, attrev;
	int i;
	
	ofp = fopen( ofile, "w+");
	
	if( ofp == NULL )
	{
		fprintf( stderr, "linker fatal: Cannot open output file %s\n", ofile );
		return 1;
	}
	
	_crc[0] = 0xFF;		/* CRC */
	_crc[1] = 0xFF;
	_crc[2] = 0xFF;
	headerParity = 0;

	/* Start Generating Module */
	/* Module signature */
	fputc(0x87, ofp);
	fputc(0xCD, ofp);
	compute_crc(0x87);
	compute_crc(0xCD);
	headerParity ^= 0x87;
	headerParity ^= 0xCD;
	
	if( omitC )
	{
		moduleSize = 14 						/* module header */
				   + strlen( modname )			/* module name */
				   + t_code						/* Code size of all segements */
				   + t_idpd                     /* Initialized direct page data of all segements */
				   + t_idat						/* Initialized data of all segments */
				   + 3;							/* CRC bytes */
	}
	else
	{
		moduleSize = 14 						/* module header */
				   + strlen( modname )			/* module name */
				   + t_code						/* Code size of all segements */
				   + t_idpd                     /* Initialized direct page data of all segements */
				   + 2							/* Linker direct page initialized data */
				   + t_idat						/* Initialized data of all segments */
				   + 2							/* Linker initialized data */
				   + 2 + t_dt * 2  				/* Data-text reference table */
				   + 2 + t_dd * 2  				/* Data-data reference table */
				   + strlen( modname ) + 1		/* Program name (NULL terminated) */
				   + 3;							/* CRC bytes */
	}
	
	if( moduleSize > 0xffff )
	{
		fprintf( stderr, "linker fatal: Module size to big (%d)\n", moduleSize );
		return 1;
	}
	
	fputc(moduleSize >> 8, ofp);
	fputc(moduleSize & 0xFF, ofp);
	compute_crc(moduleSize >> 8);
	compute_crc(moduleSize & 0xFF);
	headerParity ^= moduleSize >> 8;
	headerParity ^= moduleSize & 0xFF;
	
	/* Write module name offset */
	nameOffset = 0x0D;
	fputc(nameOffset >> 8, ofp);
	fputc(nameOffset & 0xFF, ofp);
	compute_crc(nameOffset >> 8);
	compute_crc(nameOffset & 0xFF);
	headerParity ^= nameOffset >> 8;
	headerParity ^= nameOffset & 0xFF;

	/* module type/lang */
	if( B09EntPt != NULL )
		typelang = 0x21;
	else
		typelang = 0x11;
		
	fputc(typelang, ofp);
	compute_crc(typelang);
	headerParity ^= typelang;
		
	/* module attr/rev */
	attrev = 0x81;
	fputc(attrev, ofp);
	compute_crc(attrev);
	headerParity ^= attrev;
		
	/* header check */
	headerParity = ~headerParity;
	fputc(headerParity, ofp);
	compute_crc(headerParity);
		
	/* execution offset */
	if( B09EntPt == NULL )
		execOffset = (*ob_start)->hd.h_entry + 14 + strlen( modname );
	else
		execOffset = getsym( *ob_start, B09EntPt, NULL );

	fputc(execOffset >> 8, ofp);
	fputc(execOffset & 0xFF, ofp);
	compute_crc(execOffset >> 8); compute_crc(execOffset & 0xFF);

	/* Compute data size */
	dataSize = t_stac + t_idat + t_udat + t_idpd + t_udpd + extramem;
	fputc(dataSize >> 8, ofp);
	fputc(dataSize & 0xFF, ofp);
	compute_crc(dataSize >> 8);
	compute_crc(dataSize & 0xFF);

	/* module name */
	for (i = 0; i < strlen(modname) - 1; i++)
	{
		fputc(modname[i], ofp);
		compute_crc(modname[i]);
	}
	fputc(modname[i] | 0x80, ofp);
	compute_crc(modname[i] | 0x80);
		
	/* edition */
	if( edition == -1 )
		edition = (*ob_start)->hd.h_edit;
		
	fputc(edition, ofp);
	compute_crc(edition);

	/* Now dump all of the code */
	
	ob_cur = *ob_start;
	
	while( ob_cur != NULL )
	{
		char *data;
		unsigned count;
		
		DBGPNT(( "Object %s is %4.4lx - %4.4lx\n", ob_cur->modname, ftell(ofp), ftell(ofp)+ob_cur->hd.h_ocode ));
		
		fseek( ob_cur->fp, ob_cur->object, SEEK_SET );
		data = malloc( ob_cur->hd.h_ocode );
		if( data == NULL )
		{
			fprintf( stderr, "linker fatal: Out of memory\n" );
			return 1;
		}
		
		fread( data, ob_cur->hd.h_ocode, 1, ob_cur->fp);

		/* Now patch binary */
		fseek( ob_cur->fp, ob_cur->object + ob_cur->hd.h_ocode + ob_cur->hd.h_data + ob_cur->hd.h_ddata, SEEK_SET );
		count = getwrd( ob_cur->fp );

		if( count > 0 )
		{
			DBGPNT(( "External References:\n" ));
		}
			
		while( count-- )
		{
			char	symbol[SYMLEN+1], valueflg;
			unsigned number, value;
			
			getname( symbol, ob_cur->fp );
			value = getsym( *ob_start, symbol, &valueflg );
			number = getwrd( ob_cur->fp );
			
			DBGPNT(( "%-10s %-10s %4.4x (", ob_cur->modname, symbol, value ));
			ftext( valueflg, DEF );
			DBGPNT(( ") " ));
			
			while( number-- )
			{
				unsigned flag;
				unsigned offset, result, scratch;
				flag = getc( ob_cur->fp );
				offset = getwrd( ob_cur->fp );

				if( flag & CODLOC )
				{
					DBGPNT(( " External ref patch: (" ));
					ftext( flag, REF );
					
					if( offset > ob_cur->hd.h_ocode )
					{
						fprintf( stderr, "linker fatal: Code external reference offset greater than code size\n" );
						return 1;
					}
					
					if( flag & LOC1BYT )
						scratch = data[offset];
					 else
						scratch = (unsigned)(data[offset]<<8) + data[offset+1];
						
					DBGPNT(( ") %4.4x (%4.4x) data: %4.4x, ", offset + ob_cur->Code, offset, scratch ));
					
					if( flag & NEGMASK )
						result = ~value;
					else
						result = value;
						
					switch( flag & ~(LOC1BYT|NEGMASK) )
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
							fprintf ( stderr, "fatal error: Unknown external reference flag %2.2x\n", flag );
							return 1;
							break;
					}
					
					if( flag & LOC1BYT )
						data[offset] = result;
					else
					{
						data[offset] = result >> 8;
						data[offset+1] = result & 0xff;
					}
				}
				
			}
			
			DBGPNT(( "\n" ));

		}
		
		/* Patch local refs */
		
		count = getwrd( ob_cur->fp );
		while( count-- )
		{
			unsigned flag;
			unsigned offset, result;
			
			flag = getc( ob_cur->fp );
			offset = getwrd( ob_cur->fp );
			
			if( flag & CODLOC )
			{
				if( offset > ob_cur->hd.h_ocode )
				{
					fprintf( stderr, "linker fatal: Code local reference offset greater than code size\n" );
					return 1;
				}
				
				if( flag & LOC1BYT )
					result = data[offset];
				else
					result = (unsigned)(data[offset]<<8) + data[offset+1];
	
				if( flag & NEGMASK )
					result = ~result;
				else
					result = result;
				
				if( flag & DIRENT )
				{
					if( flag & INIENT )
						result += ob_cur->IDpD;
					else
						result += ob_cur->UDpD;
				}
				else
				{
					if( flag & INIENT )
						result += ob_cur->IDat;
					else
						result += ob_cur->UDat;
				}
				
				if( flag & LOC1BYT )
					data[offset] = result;
				else
				{
					data[offset] = result >> 8;
					data[offset+1] = result & 0xff;
				}
	
				DBGPNT(( " Local ref patch (" ));
				ftext( flag, DEF|REF );
				DBGPNT(( ") %4.4x (%4.4x)\n", offset + ob_cur->Code, offset ));
			}
		}
		
		fwrite( data, ob_cur->hd.h_ocode, 1, ofp );
		buffer_crc( data, ob_cur->hd.h_ocode );
		free( data );
		
		ob_cur = ob_cur->next;
	}
		
	/* Now dump all of the Initialized DP data */
	
	ob_cur = *ob_start;
	
	while( ob_cur != NULL )
	{
		char *data;
		unsigned count;
		
		DBGPNT(( "Initialized DP data %s is %4.4lx - %4.4lx\n", ob_cur->modname, ftell(ofp), ftell(ofp)+ob_cur->hd.h_ddata ));

		fseek( ob_cur->fp, ob_cur->object + ob_cur->hd.h_ocode + ob_cur->hd.h_data, SEEK_SET );
		data = malloc( ob_cur->hd.h_ddata );
		if( data == NULL )
		{
			fprintf( stderr, "linker fatal: Out of memory\n" );
			return 1;
		}
		
		fread( data, ob_cur->hd.h_ddata, 1, ob_cur->fp);

		/* Adjust local references */
		fseek( ob_cur->fp, ob_cur->locref, SEEK_SET );
		
		count = getwrd( ob_cur->fp );
		while( count-- )
		{
			unsigned flag;
			unsigned offset, result;
			
			flag = getc( ob_cur->fp );
			offset = getwrd( ob_cur->fp );
			
			if( flag & CODLOC )
			{}
			else
			{
				if( flag & DIRLOC )
				{
					if( flag & LOC1BYT )
						result = data[offset];
					else
						result = (unsigned)(data[offset]<<8) + data[offset+1];
		
					if( flag & NEGMASK )
						result = ~result;
					else
						result = result;

					if( flag & CODENT )
					{
						result += ob_cur->Code;
					}
					else
					{
						if( flag & DIRENT )
						{
							if( flag & INIENT )
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
							if( flag & INIENT )
							{
								result += ob_cur->IDat;
							}
							else
							{
								result += ob_cur->UDat;
							}
						}
					}

					if( flag & LOC1BYT )
						data[offset] = result;
					else
					{
						data[offset] = result >> 8;
						data[offset+1] = result & 0xff;
					}
				}
			}
		}

		fwrite( data, ob_cur->hd.h_ddata, 1, ofp );
		buffer_crc( data, ob_cur->hd.h_ddata );
		free( data );
		
		/* Dump special linker initialized dp data */
		if( ob_cur == *ob_start && !omitC )
		{
			DBGPNT(( "Initialized linker dp data is %4.4lx - %4.4lx\n", ftell(ofp), ftell(ofp)+2 ));
			fputc(t_idpd, ofp);
			compute_crc(t_idpd);
			fputc(t_udpd, ofp);
			compute_crc(t_udpd);
		}

		ob_cur = ob_cur->next;
	}
		
	/* Now dump all of the Initialized data */
	
	ob_cur = *ob_start;
	
	while( ob_cur != NULL )
	{
		char *data;
		unsigned count;
		
		DBGPNT(( "Initialized data %s is %4.4lx - %4.4lx\n", ob_cur->modname, ftell(ofp), ftell(ofp)+ob_cur->hd.h_data ));
		fseek( ob_cur->fp, ob_cur->object + ob_cur->hd.h_ocode, SEEK_SET );
		data = malloc( ob_cur->hd.h_data );
		if( data == NULL )
		{
			fprintf( stderr, "linker fatal: Out of memory\n" );
			return 1;
		}
		
		fread( data, ob_cur->hd.h_data, 1, ob_cur->fp);

		/* Adjust local references */
		fseek( ob_cur->fp, ob_cur->locref, SEEK_SET );
		
		count = getwrd( ob_cur->fp );
		while( count-- )
		{
			unsigned flag;
			unsigned offset, result;
			
			flag = getc( ob_cur->fp );
			offset = getwrd( ob_cur->fp );
			
			if( flag & CODLOC )
			{}
			else
			{
				if( flag & DIRLOC )
				{
				}
				else
				{
					if( flag & LOC1BYT )
						result = data[offset];
					else
						result = (unsigned)(data[offset]<<8) + data[offset+1];
		
					if( flag & NEGMASK )
						result = ~result;
					else
						result = result;

					if( flag & CODENT )
					{
						result += ob_cur->Code;
					}
					else
					{
						if( flag & DIRENT )
						{
							if( flag & INIENT )
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
							if( flag & INIENT )
							{
								result += ob_cur->IDat;
							}
							else
							{
								result += ob_cur->UDat;
							}
						}
					}

					if( flag & LOC1BYT )
						data[offset] = result;
					else
					{
						data[offset] = result >> 8;
						data[offset+1] = result & 0xff;
					}
				}
			}
		}

		fwrite( data, ob_cur->hd.h_data, 1, ofp );
		buffer_crc( data, ob_cur->hd.h_data );
		free( data );
		
		/* Dump special linker initialized data */
		if( ob_cur == *ob_start && !omitC )
		{
			DBGPNT(( "Initialized linker data is %4.4lx - %4.4lx\n", ftell(ofp), ftell(ofp)+2 ));
			fputc(t_idat>>8, ofp);
			compute_crc(t_idat>>8);
			fputc(t_idat&0xff, ofp);
			compute_crc(t_idat&0xff);
		}

		ob_cur = ob_cur->next;
	}
	
	DBGPNT(( "Data-text table is %4.4lx - %4.4lx\n", ftell(ofp), ftell(ofp)+2+(t_dt*2) ));

	/* Now dump Data-text table */
	fputc(t_dt>>8, ofp);
	compute_crc(t_dt>>8);
	fputc(t_dt&0xff, ofp);
	compute_crc(t_dt&0xff);

	ob_cur = *ob_start;
	while( ob_cur != NULL )
	{
		unsigned count;
		
		fseek( ob_cur->fp, ob_cur->locref, SEEK_SET );
		
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
				if( flag & CODENT )
				{
					if( !omitC )
					{
						fprintf( stderr, "linker fatal: Data-text tables not allowed in non C based modules\n" );
						return 1;
					}
					
					if( flag & DIRLOC )
						offset += ob_cur->IDpD;
					else
						offset += ob_cur->IDat;
					
					fputc(offset>>8, ofp);
					compute_crc(offset>>8);
					fputc(offset&0xff, ofp);
					compute_crc(offset&0xff);
				}
				else
				{}
			}
		}

		ob_cur = ob_cur->next;
	}

	DBGPNT(( "Data-data table is %4.4lx - %4.4lx\n", ftell(ofp), ftell(ofp)+2+(t_dd*2) ));

	/* Now dump Data-data table */
	fputc(t_dd>>8, ofp);
	compute_crc(t_dd>>8);
	fputc(t_dd&0xff, ofp);
	compute_crc(t_dd&0xff);
	
	ob_cur = *ob_start;
	while( ob_cur != NULL )
	{
		unsigned count;
		
		fseek( ob_cur->fp, ob_cur->locref, SEEK_SET );
		
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
				if( flag & CODENT )
				{}
				else
				{
					if( !omitC )
					{
						fprintf( stderr, "linker fatal: Data-data tables not allowed in non C based modules\n" );
						return 1;
					}

					if( flag & DIRENT )
						offset += ob_cur->IDpD;
					else
						offset += ob_cur->IDat;
						
					fputc(offset>>8, ofp);
					compute_crc(offset>>8);
					fputc(offset&0xff, ofp);
					compute_crc(offset&0xff);
				}
			}
		}

		ob_cur = ob_cur->next;
	}

	if( !omitC )
	{
		DBGPNT(( "Program name is %4.4lx - %4.4lx\n", ftell(ofp), ftell(ofp)+strlen( modname )+1 ));
		/* Now dump program name as a C string */
		fwrite( modname, strlen( modname ), 1, ofp );
		buffer_crc( modname, strlen( modname ) );
		fputc(0, ofp);
		compute_crc(0);
	}
	
	/* Now write CRC */
	fputc(~_crc[0], ofp);
	fputc(~_crc[1], ofp);
	fputc(~_crc[2], ofp);
	
	return 0;
}

int compute_crc(a)
unsigned a;
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

int buffer_crc( data, size )
char data[];
unsigned size;
{
	register int i;
	for( i=0; i<size; i++ )
		compute_crc( data[i] );
	
	return 0;
}

/* Get value and flag for symbol */
unsigned getsym( ob, symbol, flag )
struct ob_files *ob;
char *symbol;
char *flag;
{
	while( ob != NULL )
	{
		struct exp_sym *exp;
		
		exp = ob->symbols;
		
		while( exp != NULL )
		{
			if( strcmp( symbol, exp->name ) == 0 )
			{
				if( flag != NULL ) *flag = exp->flag;
				return exp->offset;
			}
			
			exp = exp->next;
		}
		
		ob = ob->next;
	}
	
	fprintf( stderr, "linker fatal: Could not find requested symbol: %s\n", symbol );
	exit( 1 );
	
	return 0;
}



