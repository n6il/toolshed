#ifdef __cplusplus
extern "C" {
#endif

/*
 * Symbol definition/reference type/location
 */

/* type flags */
#define CODENT		0x04        /* data/code flag */

/* Code type flags */
#define CONENT		0x02		/* Constant */
#define SETENT		0x01		/* Constant */

/* data type flags */
#define DIRENT		0x02        /* global/direct flag */
#define INIENT		0x01        /* clear/init. data flag */

/* location flags */
#define LOC1BYT		0x08        /* two/one byte size flag */
#define DIRLOC		0x10        /* global/direct flag */
#define CODLOC		0x20        /* data/code flag */
#define LOCMASK		(CODLOC|DIRLOC)
#define NEGMASK		0x40        /* negate on resolution */
#define RELATIVE	0x80        /* relative reference */
#ifdef __BIG_ENDIAN__
#define ROFSYNC		0x62CD2387
#else
#define ROFSYNC		0x8723CD62 /* Reverse for Big-Endian system */
#endif
//#define SYMLEN		9         /* maximum symbol length */
#define SYMLEN		64        /* maximum symbol length */
#define MAXNAME		16        /* maximum "module" name length */


/* ROF "module" header structure */

typedef struct
{        
	/* Sync bytes used by the linker to recognize a ROF */
	int				h_sync;

	/* Type/language byte and attribute/revision byte obtained
	 * from the psect line of the module.  Only meaningful on
	 * a root psect.
	 */
	unsigned short	h_tylan;

	/* Assembly valid, used to prevent the linker from linking
	 * erroneous modules.  It is non-zero if assembly errors
	 * have occurred.
	 */
	char		h_valid;

	/* Date/Time Assembled (in OS-9/6809 date/time format) */
	char		h_date[5];

	/* Edition number.  A user definable edition number to place
	 * in the output module fr root psects.  For non-root psects,
	 * this is informational only.
	 */
	char		h_edit;

	/* Unused. */
	char		h_spare;        

	/* Size of global (or static) storage.  This value informs the
	 * linker of the amount of static data storage to reserve
	 * for the module.  The size is determined by the total size
	 * of all rmb directives in the vsects.
	 */
	unsigned short	h_glbl,

	/* Size of direct page global storage.  This value informs the
	 * linker of the amount of static direct page data storage to
	 * reserve for the module.  The size is determined by the total
	 * size of all rmb directives in the vsects.
	 */
			h_dglbl,

	
	/* Size of initialized data.  This value informs the linker
	 * of the amount of initialized data contained in the module.
	 */
			h_data,

	/* Size of direct page initialized data.  This value informs the
	 * linker of the amount of initialized data contained in the module.
	 */
			h_ddata,

	/* Size of object code.  This value is determined from the size of
	 * the assembled code.
	 */
			h_ocode;

	/* Size of stack required.  This value informs the linker of the
	 * amountfost ack space the module requires.  This value is obtained
	 * directly from the psect directive.
	 */
	unsigned short	h_stack,

	/* Offset to entry point in the object code, relative to the start
	 * of the module.  This value is obtained directly from the psect
	 * directive.
	 */
			h_entry;
} binhead;



/* Definition/reference structure */

typedef struct
{        
	char		r_flag;		/* type/location flag */
	unsigned short	r_offset;	/* reference offset */
} def_ref;

#ifdef __cplusplus
}
#endif
