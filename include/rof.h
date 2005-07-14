/* symbol definition/reference type/location */
/* type flags */
#define CODENT      0x04        /* data/code flag */
/* data type flags */
#define DIRENT      0x02        /* global/direct flag */
#define INIENT      0x01        /* clear/init. data flag */
/* location flags */
#define CODLOC      0x20        /* data/code flag */
#define DIRLOC      0x10        /* global/direct flag */
#define LOC1BYT     0x08        /* two/one byte size flag */
#define LOCMASK     (CODLOC|DIRLOC)
#define NEGMASK     0x40        /* negate on resolution */
#define RELATIVE    0x80        /* relative reference */
/* misc. constants */
/*#define ROFSYNC     0x62CD2387*/
#define ROFSYNC	    0x8723CD62 /* Reverse for Big-Endian system */
#define SYMLEN      9         /* Length of symbols */
#define MAXNAME     16        /* length of module name */

/* definition/reference */
typedef struct
{
     char r_flag; /* type/location */
     unsigned short r_offset;
} def_ref;


/* rof header structure */
typedef struct
{
     int h_sync;
     unsigned short h_tylan;
     char h_valid;
     char h_date[5];
     char h_edit;
     char h_spare;
     unsigned short h_glbl;
     unsigned short h_dglbl;
     unsigned short h_data;
     unsigned short h_ddata;
     unsigned short h_ocode;
     unsigned short h_stack;
     unsigned short h_entry;
} binhead;
