/*
  The following program lets one split a Microware C library file into its
  component pieces. This has various uses, and in particular lets one insert
  one's own versions of functions. (The header file at the beginning is one
  that I use continually, out of revulsion at the thought of using ints for
  boolean quantities. At least I can hide the fact...)

                                                Down with Funky Stuff,
                                                James Jones
 */

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef int bool;

#define TRUE    1
#define FALSE   0

/*
 * LibSplit -- split library files into their component "modules"
 *
 * usage: LibSplit libfile [module ...]
 *
 * semantics: LibSplit will read the specified library file and
 * split out its component "modules." If "module" names are
 * explicitly given on the command line, then only those "modules"
 * will be extracted; otherwise, all "modules" from the library
 * file will be extracted. Each selected "module" is written into
 * a separate file named after the "module."
 */

#include <stdio.h>

/*
 * We are only concerned with library file structure inasmuch as it
 * permits us to chop a file into its components. Thus you will not
 * see here as much detail as in, say, rdump; only the needed stuff.
 */

#define ROFSYNC 0x62cd2387      /* ROF "sync bytes" */
                                /* ("magic number" to Unixoids) */
#define SYMLEN  9               /* maximum symbol length */
#define MAXNAME 16              /* maximum "module" name length */

/* definition/reference */

typedef struct {
        uint8_t            r_flag; /* type/location */
        uint16_t        r_offset;
} __attribute__((packed)) def_ref;

/* "module" header structure */

typedef struct {
        uint32_t        h_sync;         /* should == ROFSYNC */
        uint16_t        h_tylan;        /* type/language/attr/revision */
        uint8_t            h_valid;        /* asm valid? */
        uint8_t            h_date[5];      /* creation date */
        uint8_t            h_edit;         /* edition # */
        uint8_t            h_spare;
                                        /* next, sizes of: */
        uint16_t        h_glbl,         /* globals */
                        h_dglbl,        /* direct page globals */
                        h_data,         /* data */
                        h_ddata,        /* direct page data */
                        h_ocode;        /* code */
        uint16_t        h_stack,
                        h_entry;
}       binhead;

binhead header;
FILE    *LibFP, *ModFP;
bool    GetCurr;

void CopyGlobalDefs();
void CopyCode();
void CopyExtRefs();
void CopyRefs();
void GetName();
void PutName();

int getw_os9();
int putw_os9();

int main(argc, argv)
int     argc;
char    *argv[];
{
        int     i, NMods;
        bool    GetAll;
        char    ModName[MAXNAME + 1];
        char    *LibFName, zeros;

        if (argc < 2) {
                fprintf(stderr, "usage: LibSplit libfile [module ...]\n");
                exit(1);
        }
        if ( (GetAll = (argc == 2)) )
                NMods = 30000;
        else
                NMods = argc - 2;

        LibFName = argv[1];

        if ((LibFP = fopen(LibFName, "r")) == NULL) {
                fprintf(stderr, "LibSplit: can't open %s\n", LibFName);
                exit(1);
        }
		
        while (NMods > 0) {
        		
        		/* Skip past any leading zeros */
        		zeros = getc( LibFP );
        		if( zeros != 0 )
        			ungetc( zeros, LibFP );
        		else
        			zeros = getc( LibFP );
        			
                if (fread(&header, sizeof(header), 1, LibFP) < 1)
                        break;
                if (header.h_sync != ROFSYNC) {
                        fprintf(stderr, "%s is not a library file\n", LibFName);
                        exit(1);
                }

                GetName(ModName);
                
                if (GetAll)
                        GetCurr = TRUE;
                else {
                        GetCurr = FALSE;
                        for (i = 2; i < argc; i++) {
                                if (strcmp(ModName, argv[i]) == 0) {
                                        GetCurr = TRUE;
                                        break;
                                }
                        }
                }

                if (GetCurr) {
                        NMods--;
                        if ((ModFP = fopen(ModName, "w")) == NULL) {
                                fprintf(stderr, "LibSplit: can't create %s\n", ModName);
                                exit(1);
                        }
                        
                fwrite(&header, sizeof(header), 1, ModFP);
                        PutName(ModName);
                }

                CopyGlobalDefs();
                CopyCode();
                CopyExtRefs();
                CopyRefs();     /* local references */

                if (GetCurr)
                        fclose(ModFP);
        }

        fclose(LibFP);
	return 0;
}

void CopyGlobalDefs()
{
        int     GCount, Offset;
        char    GSym[SYMLEN + 1], flag;

        GCount = getw_os9(LibFP);
        if (GetCurr)
                putw_os9(GCount, ModFP);

        for (; GCount > 0; GCount--) {
                GetName(GSym);
                if (GetCurr) {
                        PutName(GSym);
                        putc((flag = getc(LibFP)), ModFP);
                        putw_os9((Offset = getw_os9(LibFP)), ModFP);
                        } else
                        fseek(LibFP, (long)(sizeof(uint8_t) + sizeof(uint16_t)), 1);
        }
}

void CopyCode()
{
        int     HowMuch;
        char    buffer[BUFSIZ];

        HowMuch = header.h_ocode + header.h_ddata + header.h_data;

        if (GetCurr) {
                for (; HowMuch >= BUFSIZ; HowMuch -= BUFSIZ) {
                        fread(buffer, 1, BUFSIZ, LibFP);
                        fwrite(buffer, 1, BUFSIZ, ModFP);
                }

                if (HowMuch > 0) {
                        fread(buffer, 1, HowMuch, LibFP);
                        fwrite(buffer, 1, HowMuch, ModFP);
                }
        } else
                fseek(LibFP, (long) HowMuch, 1);
}

void CopyExtRefs()
{
        int     ECount;
        char    ESym[SYMLEN + 1];
		
		ECount = getw_os9(LibFP);
        if (GetCurr)
                putw_os9(ECount, ModFP);

        for (; ECount > 0; ECount--) {
                GetName(ESym);
                if (GetCurr)
                        PutName(ESym);
                CopyRefs();
        }
}

void CopyRefs()
{
        int     RCount;
        def_ref ref;

        RCount = getw_os9(LibFP);
        if (GetCurr) {
                putw_os9(RCount, ModFP);
                for (; RCount > 0; RCount--) {
                        fread(&ref, sizeof(ref), 1, LibFP);
                        fwrite(&ref, sizeof(ref), 1, ModFP);
                }
        } else
                fseek(LibFP, (long) (RCount * sizeof(ref)), 1);
}

void GetName(s)
char    *s;
{
        while ( (*(s++) = getc(LibFP)) )
                ;
}

void PutName(s)
char    *s;
{
        fputs(s, ModFP);
        putc('\0', ModFP);
}

int getw_os9( fp )
FILE *fp;
{
	unsigned int result;
	
	result = (unsigned char)getc( fp );
	result <<= 8;
	result += (unsigned char)getc( fp );
	
	return result;
}

int putw_os9(value, fp)
int value;
FILE *fp;
{
	putc( (value>>8)&0xff, fp );
	putc( (value)&0xff, fp );
	
	return 0;
}