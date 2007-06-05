/*****************************************************************************
	as.h	- Assembler wide definitions

	Copyright (c) 2004 Chet Simpson, Digital Asphyxia. All rights reserved.

	The distribution, use, and duplication this file in source or binary form
	is restricted by an Artistic License (see license.txt) included with the
	standard distribution. If the license was not included with this package
	please refer to http://www.oarizo.com for more information.


	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that: (1) source code distributions
	retain the above copyright notice and this paragraph in its entirety, (2)
	distributions including binary code include the above copyright notice and
	this paragraph in its entirety in the documentation or other materials
	provided with the distribution, and (3) all advertising materials
	mentioning features or use of this software display the following
	acknowledgement:

		"This product includes software developed by Chet Simpson"
	
	The name of the author may be used to endorse or promote products derived
	from this software without specific priorwritten permission.

	THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR IMPLIED
	WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
	MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

*****************************************************************************/

/*
 *      machine independent definitions and global variables
 */
#ifndef AS_H
#define AS_H

#include "config.h"
#include "macro.h"
#include "symtab.h"
#include "pseudo.h"
#include "cpu.h"
#include "table9.h"
#include "util.h"
#include "error.h"
#include "output.h"
#include "context.h"


typedef struct _Export {
	struct _Export	*next;
	char			*name;
} Export;


/* global variables */


/*
	various line counters
*/
extern u_int16		Cfn;			/* Current file number 1...n			*/


/*
	internal/external options
*/

extern int			Cpflag;			/* print cumulative cycles flag			*/
extern int			outputROMSize;	/* Size of the output ROM				*/
/*
	file handling pointers/counters
*/
extern u_int16		asmFileCount;	/* Number of files to assemble			*/
extern char			*filelist[];	/* list of files to assemble			*/
extern char			*ipathlist[];	/* search paths for include/lib pseudo	*/


/*
	misc pointers and other data
*/
extern char			**Argv;			/* pointer to file names				*/


/*
	Structures
*/
extern bool			instruct;
extern Struct		*laststructfound;

/* More misc */
extern char			*includeDirectories[256];
extern int			includeCount;
extern char			*outputDirectory;
extern char			*outputFilename;



#endif
