/********************************************************************
 * os9module.h - OS-9 module definitions file
 *
 * $Id$
 ********************************************************************/

#ifndef	_OS9MODULE_H
#define	_OS9MODULE_H

#include <cocotypes.h>

typedef struct osk_module_t  {
	u_char id[2];
	u_char sysrev[2];
	u_char size[4];
	u_char owner[4];
	u_char name[4];
    u_char accs[2];
	u_char type;
	u_char lang;
	u_char attr;
	u_char revs;
	u_char edit[2];
	u_char usage[4];
	u_char symbol[4];
	u_char ident[2];
	u_char reserved[3*2];
	u_char hdext[4];
	u_char ddextSz[2];
	u_char parity[2];
	union  {
		u_char  data[1];
		struct  {
			u_char  exec[4];
			u_char  excpt[4];
		} system;
		struct  {
			u_char  exec[4];
			u_char  excpt[4];
			u_char  mem[4];
			u_char  stack[4];
			u_char  idata[4];
			u_char  irefs[4];
		} program;
		struct  {
			u_char  exec[4];
			u_char  excpt[4];
			u_char  mem[4];
		} driver;
		struct  {
			u_char  exec[4];
			u_char  excpt[4];
			u_char  mem[4];
			u_char  stack[4];
			u_char  idata[4];
			u_char  irefs[4];
			u_char  init[4];
			u_char  term[4];
		} traphandler;
		struct  {
			u_char  port[4];
			u_char  vector;
			u_char  irqlvl;
			u_char  prior;
			u_char  mode;
			u_char  fmgr[2];
			u_char  pdev[2];
			u_char  devcon[2];
			u_char  reserved1[2];
			u_char  devflags[4];
			u_char  reserved2[2];
			u_char  opt[2];
			u_char  dtyp;
		} descriptor;
	} data;
} OSK_MODULE_t;

#define OSK_HEADER_SIZE 48

#define OSK_ID0 0x4A
#define OSK_ID1 0xFC


typedef struct os9_module_t
{
	u_char id[2];
	u_char size[2];
	u_char name[2];
	u_char tyla;
	u_char atrv;
	u_char parity;
	union
	{
	    u_char data[1];		/* plain modules */
   		struct
		{
			u_char exec[2];
			u_char data[1];
		} system;
		struct
		{
			u_char exec[2];
			u_char mem[2];
			u_char data[1];
		} program;
		struct
		{
			u_char exec[2];
			u_char mem[2];
			u_char mode[1];
			u_char data[1];
		} driver;
		struct
		{
			u_char exec[2];
			u_char data[1];
		} file_mgr;
		struct
		{
			u_char fmgr[2];
			u_char driver[2];
			u_char mode;
			u_char port[3];
			u_char opt;
			u_char dtype;
			u_char data[1];
		} descriptor;
	} data;
} OS9_MODULE_t;


#define OS9_HEADER_SIZE 9
#define TYPE_MASK 0xF0

typedef enum os9_type_t
{
	NULL_TYPE = 0,
	Prgrm, 
	Sbtrn, 
	Multi, 
	Data,  
	SSbtrn,
	TYPE_6,
	TYPE_7,
	TYPE_8,
	TYPE_9,
	TYPE_A,
	Traplib,
	Systm, 
	FlMgr,
	Drivr,
	Devic  
} OS9_TYPE_t;

#define LANG_MASK 0x0F
typedef enum os9_lang_t
{
	NULL_LANG = 0,
	Objct,
	ICode,
	PCode,
	CCode,
	CblCode,
	FrtnCode,
	Obj6309
} OS9_LANG_t;

#define ATTR_MASK 0xF0
typedef enum os9_attr_t
{
	ReEnt   = 0x80,
	Modprot = 0x40
} OS9_attr_t;


#define REVS_MASK 0x0F

#define OS9_ID0 0x87
#define OS9_ID1 0xCD

#define OS9_CRC0 0x80
#define OS9_CRC1 0x0F
#define OS9_CRC2 0xE3

#define INT(foo) (foo[0] * 256 + foo[1])

error_code _os9_crc_compute(u_char *ptr, u_int sz, u_char *crc);
error_code _os9_crc(OS9_MODULE_t *mod);
u_char  _os9_header(OS9_MODULE_t *mod);

error_code _osk_crc(OSK_MODULE_t *mod);
u_short _osk_header(OSK_MODULE_t *mod);

#endif /* _OS9MODULE_H */
