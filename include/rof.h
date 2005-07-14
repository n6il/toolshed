/* symbol definition/reference type/location */

/* type flags */
#define CODENT		0x04        /* data/code flag */

/* data type flags */
#define DIRENT		0x02        /* global/direct flag */
#define INIENT		0x01        /* clear/init. data flag */

/* location flags */
#define CODLOC		0x20        /* data/code flag */
#define DIRLOC		0x10        /* global/direct flag */
#define LOC1BYT		0x08        /* two/one byte size flag */
#define LOCMASK		(CODLOC|DIRLOC)
#define NEGMASK		0x40        /* negate on resolution */
#define RELATIVE	0x80        /* relative reference */
#ifdef __BIG_ENDIAN__
#define ROFSYNC		0x62CD2387
#else
#define ROFSYNC		0x8723CD62 /* Reverse for Big-Endian system */
#endif
#define SYMLEN		9         /* maximum symbol length */
#define MAXNAME		16        /* maximum "module" name length */


/* ROF "module" header structure */

typedef struct
{        
long		h_sync;         /* should == ROFSYNC */
unsigned	h_tylan;        /* type/language/attr/revision */
char		h_valid;        /* asm valid? */ 
char		h_date[5];      /* creation date */
char		h_edit;         /* edition # */
char		h_spare;        
                                        /* next, sizes of: */
unsigned	h_glbl,         /* globals */
		h_dglbl,        /* direct page globals */
		h_data,         /* data */
		h_ddata,        /* direct page data */
		h_ocode;        /* code */
unsigned	h_stack,
		h_entry;

} binhead;



/* definition/reference */

typedef struct
{        
	char		r_flag;		/* type/location */
	unsigned	r_offset;

} def_ref;

