/***************************************************************************
 * mamou.h: main header file
 *
 * $Id$
 *
 * The Mamou Assembler - A Hitachi 6309 assembler
 *
 * (C) 2004 Boisy G. Pitre
 ***************************************************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#ifndef BDS
#include <unistd.h>
#endif
#include <ctype.h>
#include <sys/types.h>

#include "cocopath.h"

#define VERSION_MAJOR   1
#define VERSION_MINOR   0

#define ERR     (-1)


/* Variable limits */

#define FNAMESIZE	512
#define MAXBUF		1024
#define MAXOP		10      /* longest mnemonic */
#define MAXLAB		32
#define E_LIMIT		16
#define P_LIMIT		64


/* Character Constants */

#define TAB     '\t'
#define BLANK   ' '
#define EOS     '\0'


/* Opcode Classes */

typedef enum _opcode_class
{
     INH,			/* Inherent                     */
     GEN,			/* General Addressing           */
     IMM,			/* Immediate only               */
     REL,			/* Short Relative               */
     P2REL,			/* Long Relative                */
     P1REL,			/* Long Relative (LBRA and LBSR)*/
     NOIMM,			/* General except for Immediate */
     P2GEN,			/* Page 2 General               */
     P3GEN,			/* Page 3 General               */
     RTOR,			/* Register To Register         */
     INDEXED,		/* Indexed only                 */
     RLIST,			/* Register List                */
     P2NOIMM,		/* Page 2 No Immediate          */
     P2INH,			/* Page 2 Inherent              */
     P3INH,			/* Page 3 Inherent              */
     GRP2,			/* Group 2 (Read/Modify/Write)  */
     LONGIMM,		/* Immediate mode takes 2 bytes */
     BTB,			/* Bit test and branch          */
     SETCLR,			/* Bit set or clear             */
     CPD,			/* compare d               6811 */
     XLIMM,			/* LONGIMM for X           6811 */
     XNOIMM,			/* NOIMM for X             6811 */
     YLIMM,			/* LONGIMM for Y           6811 */
     YNOIMM,			/* NOIMM for Y             6811 */
     FAKE,			/* convenience mnemonics   6804 */
     APOST,			/* A accum after opcode    6804 */
     BPM,			/* branch reg plus/minus   6804 */
     CLRX,			/* mvi x,0                 6804 */
     CLRY,			/* mvi y,0                 6804 */
     LDX,			/* mvi x,expr              6804 */
     LDY,			/* mvi y,expr              6804 */
     MVI,			/* mvi                     6804 */
     EXT,			/* extended                6804 */
     BIT,			/* bit manipulation        6301 */
     SYS,			/* syscalls (really swi)        */
//     PSEUDO,			/* Pseudo ops                   */
     P2RTOR,			/* Page 2 register to register  */
     P3RTOR,			/* Page 3 register to register  */
     P3IMM,			/* Page 3 immediate only	*/
     P3NOIMM			/* Page 3 No Immediate          */
} opcode_class;



/* Pseudo-op classes. */

typedef enum _pseudo_class
{
	RM,              /* Reserve Memory               */
	FC,              /* Form Constant                */
	ORG,             /* Origin                       */
	EQU,             /* Equate                       */
	IF,				 /* Start of conditional		 */
	ENDC,            /* End of condtional			 */
	ELSE,			 /* else from conditional		 */
	OTHER			 /* all other pseudo-ops		 */
} pseudo_class;


struct filestack
{
	coco_path_id	fd;
	char			file[FNAMESIZE];
	u_int			current_line;
	int		num_blank_lines;
	int		num_comment_lines;
	int			end_encountered;
};



/* linked list to hold line numbers */

struct link
{
	int		L_num; /* line number */
	struct link		*next; /* pointer to next node */
};

struct nlist
{	/* basic symbol table entry */
	char			*name;
	int		def;
	int			overridable;
	struct nlist	*Lnext ; /* left node of the tree leaf */
	struct nlist	*Rnext; /* right node of the tree leaf */ 
	struct link		*L_list; /* pointer to linked list of line numbers */
};



struct psect
{
	int		org;
	int		size;
	struct psect	*next;
};




typedef enum
{
	OPCODE_H6309,
	OPCODE_PSEUDO,
	OPCODE_UNKNOWN
} opcode_type;


/* mnemonic table entry */

enum {
	CPU_M6809=0,
	CPU_H6309=1,
	CPU_X9=2
};

struct h6309_opcode
{
	char    *mnemonic;      /* its name */
	char    class;          /* its class */
	int     opcode;         /* its base opcode */
	char    cycles;         /* its base # of cycles */
	char    cpuclass;       /* its processor class (CPU_* above) */
	int	(*func)();	/* function */
};


typedef enum
{
	HAS_NO_OPERAND = 0,
	HAS_OPERAND,
	HAS_OPERAND_WITH_SPACES,
	HAS_OPERAND_WITH_DELIMITERS
} pseudo_info;


struct pseudo_opcode
{
	char			*pseudo;        /* its name */
	pseudo_class	class;
	pseudo_info		info;
	int				(*func)();		/* function */
};



typedef struct
{
	opcode_type	type;
	union
	{
		struct h6309_opcode		*h6309;
		struct pseudo_opcode	*pseudo;
	} opcode;
} mnemonic;


typedef enum
{
	LINETYPE_COMMENT,
	LINETYPE_BLANK,
	LINETYPE_SOURCE
} line_type;


struct source_line
{
	line_type			type;
	int				has_warning;				/* allow assembler warnings */
	char				label[MAXLAB];				/* label on current line */
	char				Op[MAXOP];					/* opcode mnemonic on current line */
	char				operand[MAXBUF];			/* remainder of line after op */
	char				comment[MAXBUF];			/* comment after operand, or entire line */
	mnemonic			mnemonic;
	char				*optr;						/* pointer into current operand field */
	int				force_word;					/* Result should be a word when set */
	int				force_byte;					/* result should be a byte when set */
};



/* Object generation types. */

typedef enum
{
	OUTPUT_BINARY,
	OUTPUT_HEX,
	OUTPUT_SRECORD
} object_file_type;



/* Assembly modes. */

typedef enum
{
	ASM_OS9,
	ASM_DECB,
	ASM_ROM
} asm_mode;



/* Assembler state */

typedef struct _assembler
{
	time_t				start_time;
	struct source_line  line;						/* current source line */
	object_file_type	output_type;				/* type of output file */
	u_int			num_errors;					/* total number of errors */
	u_int			num_warnings;				/* assembler warnings */
	u_int			cumulative_blank_lines;		/* blank line count across all files */
	u_int			cumulative_comment_lines;   /* comment line count across all files */
	u_int			cumulative_total_lines;		/* total line count across all files */
	char				input_line[MAXBUF];			/* input line buffer */
	u_int			program_counter;			/* Program Counter */
	u_int			data_counter;				/* data counter */
	u_int			old_program_counter;		/* Program Counter at beginning */
	u_int			DP;							/* Direct Page pointer */
	u_int			last_symbol;				/* result of last symbol_find */
	u_int			pass;						/* current pass */
	struct filestack	*current_file;
	u_int			use_depth;					/* depth of includes/uses */
#define MAXAFILE	16
	u_int			file_index;
	char				*file_name[MAXAFILE];		/* assembly file name on cmd line */
	u_int			current_filename_index;		/* file number count            */
#define INCSIZE 16
	u_int			include_index;
	char				*includes[INCSIZE];	
	u_int					Ffn;						/* forward ref file #           */
	u_int			F_ref;						/* next line with forward ref   */
	char				**arguments;				/* pointer to file names        */
	u_int			E_total;					/* total # bytes for one line   */
	char				E_bytes[E_LIMIT + MAXBUF];  /* Emitted held bytes           */
	u_int			E_pc;						/* Pc at beginning of collection*/
	u_int			P_force;					/* force listing line to include Old_pc */
	u_int			P_total;					/* current number of bytes collected    */
	char				P_bytes[P_LIMIT + 60];		/* Bytes collected for listing  */
	u_int			cumulative_cycles;			/* # of cycles per instruction  */
	u_int			Ctotal;						/* # of cycles seen so far */
	int				f_new_page;					/* new page flag */
	u_int			page_number;				/* page number */
	int				o_show_cross_reference;		/* cross reference table flag */
	int				f_count_cycles;				/* cycle count flag */
	u_int			Opt_C;						/* */
	int			o_page_depth;				/* page depth */
	int				o_show_error;				/* show error */
	u_int			Opt_F;						/* */
	u_int			Opt_G;						/* */
	int				o_show_listing;				/* listing flag 0=nolist, 1=list*/
	asm_mode			o_asm_mode;					/* assembler mode */
	u_int			Opt_N;						/* */
	int				o_quiet_mode;				/* quiet mode */
	int				o_show_symbol_table;		/* symbol table flag, 0=no symbol */
	u_int			o_pagewidth;				/* page width */
	u_int			current_line;				/* line counter for printing */
	u_int			current_page;				/* page counter for printing */
	u_int			header_depth;				/* page header number of lines */
	u_int			footer_depth;				/* page footer of lines */
	int				o_format_only;              /* format only flag, 0=no symbol */
	int				o_debug;					/* debug flag */
	coco_path_id		fd_object;					/* object file's file descriptor*/
	int				object_output;
	char				object_name[FNAMESIZE];
	char				_crc[3];
	u_int			do_module_crc;
	int				ignore_errors;
	int				tabbed;
#define	CONDSTACKLEN	256
	u_int			conditional_stack_index;
	char				conditional_stack[CONDSTACKLEN];
	int				o_do_parsing;
	int				o_cpuclass;
	u_int			code_bytes;					/* number of emitted code bytes */
#define NAMLEN 64
#define TTLLEN NAMLEN
	u_char				name_header[NAMLEN];
	u_char				title_header[TTLLEN];
	struct nlist		*bucket;					/* root node of the tree */
	struct psect		psect[256];
	int			current_psect;
	int				code_segment_start;
	u_int			decb_exec_address;
	int					newstyle;
	int					pseudoUppercase;
} assembler;


/* function prototypes */
/* mamou.c */
int main(int argc, char **argv);
void mamou_pass(assembler *as);
void mamou_parse_line(assembler *as, char *input_line);
void process(assembler *as);
void mamou_init_assembler(assembler *as);

/* h6309.c */
void local_init(void);

/* env.c */
void env_init(assembler *as);

/* evaluator.c */
int evaluate(assembler *as, int *result, char **eptr, int);

/* ffwd.c */
void fwd_init(assembler *as);
void fwd_deinit(assembler *as);
void fwd_mark(assembler *as);
void fwd_next(assembler *as);
void fwd_reinit(assembler *as);


/* print.c */
void print_line(assembler *as, int override, char infochar, int counter);
void print_summary(assembler *as);
void print_header(assembler *as);
void print_footer(assembler *as);


/* symbol_bucket.c */
struct nlist *symbol_add(assembler *as, char *str, int val, int override);
struct nlist *symbol_find(assembler *as, char *name, int);
int mne_look(assembler *as, char *str, mnemonic *m);
void symbol_dump_bucket(struct nlist *ptr);
void symbol_cross_reference(struct nlist *ptr);


/* util.c */
char *extractfilename(char *pathlist);
int alpha(char c);
int numeric(char c);
int alphan(char c);
int any(char c, char *str);
int delim(char c);
int eol(char c);
void decb_header_emit(assembler *as, u_int start, u_int size);
void decb_trailer_emit(assembler *as, u_int exec);
void emit(assembler *as, int byte);
void error(assembler *as, char *str);
void eword(assembler *as, int wd);
void equad(assembler *as, int qwd);
void f_record(assembler *as);
void fatal(char *str);
void finish_outfile(assembler *as);
int head(char *str1, char *str2);
int hiword(int i);
int loword(int i);
u_int hibyte(int i);
int lobyte(int i);
char mapdn(char c);
char *skip_white(char *ptr);


/* pseudo.c */
int	_else(assembler *as),
	_align(assembler *as),
	_dts(assembler *as),
	_dtb(assembler *as),
	_emod(assembler *as),
	__end(assembler *as),
	_endc(assembler *as),
	_equ(assembler *as),
	_even(assembler *as),
	_fdb(assembler *as),
	_fcb(assembler *as),
	_fcc(assembler *as),
	_fcr(assembler *as),
	_fcs(assembler *as),
	_fcn(assembler *as),
	_fcz(assembler *as),
	_fqb(assembler *as),
	_fill(assembler *as),
	_ifeq(assembler *as),
	_ifge(assembler *as),
	_ifgt(assembler *as),
	_ifle(assembler *as),
	_iflt(assembler *as),
	_ifne(assembler *as),
	_ifp1(assembler *as),
	_ifp2(assembler *as),
	_mod(assembler *as),
	_nam(assembler *as),
	_null_op(assembler *as),
	_odd(assembler *as),
	_opt(assembler *as),
	_org(assembler *as),
	_page(assembler *as),
	_rmb(assembler *as),
	_rmd(assembler *as),
	_rmq(assembler *as),
	_set(assembler *as),
	_setdp(assembler *as),
	_spc(assembler *as),
	_ttl(assembler *as),
	_use(assembler *as),
	_zmb(assembler *as),
	_zmd(assembler *as),
	_zmq(assembler *as);

/* h6309.c */
int	_gen(assembler *as, int opcode),
	_grp2(assembler *as, int opcode),
	_indexed(assembler *as, int opcode),
	_inh(assembler *as, int opcode),
	_imm(assembler *as, int opcode),
	_imgen(assembler *as, int opcode),
	_longimm(assembler *as, int opcode),
	_noimm(assembler *as, int opcode),
	_p1rel(assembler *as, int opcode),
	_p2gen(assembler *as, int opcode),
	_ldqgen(assembler *as, int opcode),
	_p2inh(assembler *as, int opcode),
	_p2noimm(assembler *as, int opcode),
	_p3noimm(assembler *as, int opcode),
	_p2rel(assembler *as, int opcode),
	_p3gen(assembler *as, int opcode),
	_p3gen8(assembler *as, int opcode),
	_p3inh(assembler *as, int opcode),
	_p3imm(assembler *as, int opcode),
	_rel(assembler *as, int opcode),
	_rlist(assembler *as, int opcode),
	_rtor(assembler *as, int opcode),
	_p2rtor(assembler *as, int opcode),
	_p3rtor(assembler *as, int opcode),
	_sys(assembler *as, int opcode);


