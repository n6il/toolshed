/*****************************************************************************
	as.c	- Assembler mainline

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
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "as.h"
#include "proto.h"
#include "os9.h"
#include "label.h"
#include "input.h"

/* global variables */

int			outputROMSize = 8192;			/* Size for the ROM file */
/*
	occurance count holders
*/


/*
	various line counters
*/
u_int16		Cfn = 0;						/* Current file number 1...n			*/



/*
	internal/external options
*/
int				Cpflag = 0;						/* print cumulative cycles flag			*/

/*
	file handling pointers/counters
*/
u_int16		asmFileCount =0;             	/* Number of files to assemble			*/
char		*filelist[MAX_FILES];			/* list of files to assemble		*/
char		*ipathlist[MAX_SEARCHPATHS];	/* search paths for include/lib pseudo */

/*
	misc pointers and other data
*/
char		**Argv =0;              		/* pointer to file names        		*/
u_int16		OptCount;						/* Counter for lines that can be optimized */
int			lineCount = 0;

char		*includeDirectories[256];
int			includeCount;
char		*outputDirectory;
char		*outputFilename;



/*
 *	as ---	cross assembler main program
 */
static void Initialize(EnvContext *ctx)
{
	atexit(FwdRefDone);

	ctx->m_CPUType = CPU_6309;		/* CPU Type									*/
	ctx->m_OutputType = MOTBIN;		/* output type.								*/
	ctx->m_Pass = 1;				/* Which pass the assembler is on			*/
	ctx->m_LineNumber = 0;			/* Line number of the current file			*/
	ctx->m_CycleCount = 0;			/* # of cycles per instruction				*/
	ctx->m_CycleTotal = 0;			/* # of cycles seen so far					*/
	ctx->m_ForceWord = false;		/* Result should be a word when set			*/
	ctx->m_ForceByte = false;		/* Result should be a byte when set			*/
	ctx->m_OptimizeLine = false;	/* Flag to indicate line can be optimized	*/
	ctx->m_Finished = false;		/* flag indicating END directive       		*/
	ctx->m_HasEntryPoint = false;	/* m_EntryPoint is valid					*/
	ctx->m_EntryPoint = 0;			/* 16 bit address to the files entry point	*/
	ctx->m_ErrorCount = 0;			/* $ of errors encountered during assembly	*/
	ctx->m_WarningCount = 0;		/* # of warnings encounted during assembly	*/
	ctx->m_OriginSet = false;		/* Has the origin been set?					*/
	ctx->m_OrgBase = 0;				/* Last ORG statement						*/
	ctx->m_DisableWarnings = false;	/* Don't disable warnings					*/

	ctx->m_Cond.m_Count = NO_CONDITION;
	ctx->m_SilentMode = false;		/* Display no output except errors and warnings */
	ctx->m_Misc.Oflag = true;					/* enable/disable output file			*/
	ctx->m_Misc.Cflag = false;					/* cycle count flag 					*/
	ctx->m_Misc.Sflag = false;					/* symbol table flag, 0=no symbol		*/
	ctx->m_Misc.N_page = false;					/* new page flag						*/
	ctx->m_Misc.OptFlag = false;				/* Display number of lines that could be optimized */
	ctx->m_Misc.OptShow = false;				/* Display lines that could be optimized */


	ctx->m_Compat.m_AsmMode = ASM_MODE_CASM;
	ctx->m_Compat.m_AsmOpMask = ASM_ALL;
	ctx->m_Compat.m_Warn = false;
	ctx->m_Compat.m_DisablePCIndex = false;
	ctx->m_Compat.m_ForcePCR = false;
	ctx->m_Compat.m_ForceZeroOffset = false;
	ctx->m_Compat.m_StrictLocals = false;
	ctx->m_Compat.m_DisableMacros = false;
	ctx->m_Compat.m_DisableLocals = false;
	ctx->m_Compat.m_IgnoreCase = false;
	ctx->m_Compat.m_UsePrecedence = false;
	ctx->m_Compat.m_SemicolonComment = true;


	ctx->m_ListingFlags.m_Title[0] = 0;			/* Set the title to 0						*/
	ctx->m_ListingFlags.m_CmdEnabled = false;	/* Enable listing							*/
	ctx->m_ListingFlags.m_OptEnabled = false;	/* Enable listing							*/
	ctx->m_ListingFlags.m_ListSymbols = false;	/* List symbols and end of assembly			*/
	ctx->m_ListingFlags.m_ExpandMacros = false;	/* Expand Macros in listing?				*/
	ctx->m_ListingFlags.m_PageNumber = 0;		/* Page number								*/
	ctx->m_ListingFlags.P_force = false;		/* force listing line to include regtenOldPC */
	ctx->m_ListingFlags.P_total = 0;			/* current number of bytes collected		*/
	ctx->m_ListingFlags.OptNoLineNumbers = false;	/* Do not print line numbers in listing	*/
	ctx->m_ListingFlags.OptNoOpData = false;	/* Do not print op data in listing			*/
	ctx->m_ListingFlags.OptCommentMacros = false;	/* Comment out macros in listing		*/


	InitSymbolTable(ctx);

	ctx->m_Misc.N_page  = false;
	Cpflag  = 0;
	Cfn = 0;
	lineCount = 0;

	includeCount = 0;
	outputDirectory = NULL;
	outputFilename = NULL;

	FwdRefInit();	/* forward ref init */
	InitCPU();
	InitOpcodeTable();
	ResetMacroLocalLabels();
	ResetLocalLabels(ctx);
}


static void Reinitialize(EnvContext *ctx)
{
	ctx->m_CycleTotal	= 0;
	ctx->m_HasEntryPoint = false;
	ctx->m_EntryPoint  = 0;
	ctx->m_LineNumber = 0;
	ctx->m_Finished = false;		/* no END directive yet */
	ctx->m_OriginSet = false;		/* Has the origin been set?					*/
	ctx->m_OrgBase = 0;				/* Last ORG statement						*/

	ctx->m_Cond.m_Count = NO_CONDITION;
	ctx->m_ListingFlags.m_Title[0] = 0;			/* Set the title to 0						*/

	ctx->m_ErrorCount = 0;
	ctx->m_WarningCount = 0;
	ctx->m_Misc.N_page  = false;
	Cpflag  = 0;
	Cfn = 0;
	lineCount = 0;

	InitCPU();
	FwdRefReinit(ctx);
	ResetMacroLocalLabels();
	ResetLocalLabels(ctx);
}


static bool GetLine(EnvContext *ctx, char *buffer, int max)
{
	/* Reset the contexts buffer information */
	ctx->m_Line = NULL;
	ctx->m_Label = NULL;
	ctx->m_Opcode = NULL;
	ctx->m_Operand = NULL;
	ctx->m_Ptr = NULL;

	if(true == IsMacroOpen() && NULL != GetMacroLine(ctx, buffer)) {
		return true;
	}

	return ReadInputLine(ctx, buffer, max);
}


static void AssembleFile(EnvContext *ctx, const char *np)
{
	LineBuffer	lineBuffer;

	
	SetNamespace(NULL, "");
	OptCount = 0;		/* Reset Optimization count */

	if(ctx->m_SilentMode == false) {
		fprintf(stderr, "Assembler pass: %d  ", ctx->m_Pass);
		if(ctx->m_Pass > 1) {
			fprintf(stderr, "Assembling %s", np);
		}
		fprintf(stderr, "\n");
	}


	while(0 != GetOpenFileCount()) {

		while(false != GetLine(ctx, lineBuffer.m_Line, MAX_BUFFERSIZE)) {
			const char *limit;
			bool opcodeENDM;

			limit = lineBuffer.m_Line + MAX_BUFFERSIZE - 1;
			opcodeENDM = false;

			if(false == IsMacroOpen()) {
				ctx->m_LineNumber++;
			}
			
			lineCount++;

			
			ctx->m_ListingFlags.P_force = false;	/* No force unless bytes emitted */
			ctx->m_Misc.N_page = false;
			ctx->m_OptimizeLine = false;	/* Set optimization alert flag to none */

			if(true == ParseLine(&lineBuffer, ctx)) {
				opcodeENDM = ProcessLine(ctx);
			}

			
			if(EOS == *lineBuffer.m_Line) {
				GetNextMacroLocalLabel();
				GetNextLocalLabel();
			}

			if(ctx->m_Pass == 2) {
				if(true == ctx->m_ListingFlags.m_OptEnabled && false == ctx->m_Misc.N_page) {
					if(false == IsMacroOpen() || true == OnFirstMacroLine() || true == ctx->m_ListingFlags.m_ExpandMacros) {
						bool comment;

						comment = false;

						if(true == ctx->m_ListingFlags.OptCommentMacros) {
							if(true == opcodeENDM || true == IsProcessingMacro() || (true == OnFirstMacroLine() && true == ctx->m_ListingFlags.m_ExpandMacros)) {
								comment = true;
							}
						}

						PrintLine(ctx, lineBuffer.m_Line, comment);

					}
				} else if(ctx->m_OptimizeLine == true) {
					if(ctx->m_Misc.OptShow == true) {
						warning(ctx, WARN_OPTIMIZE, "!!Optimization alert!! ");
						PrintLine(ctx, lineBuffer.m_Line, false);
					}
					OptCount++;
				}
			}

			if(Cpflag == 3) {
				PrintCycles(ctx, ctx->m_CycleTotal);		/* print cumulative cycles */
			}


			ctx->m_ListingFlags.P_total = 0;			/* reset byte count, */
			Cpflag = 0;				/* cycle print flag, */
			ctx->m_CycleCount = 0;			/* and per instruction cycle count */


			if(false != ctx->m_Finished) {
				while(0 != GetOpenFileCount()) {
					CloseInputFile(ctx);
				}
				break;
			}
		}

		if(0 != GetOpenFileCount()) {
			CloseInputFile(ctx);
		}
	}
	FlushOutput(ctx);
}





int main(int argc, char **argv)
{
	EnvContext	ctx;
	char		**np;
#ifdef _WIN32
	DWORD	startTime;
	DWORD	endTime;
#endif

	Argv = argv;



	Initialize(&ctx);

	Params(&ctx, argc, argv);

	np = filelist;
	Cfn = 0;

#ifdef _WIN32
	startTime = GetTickCount();
#endif

	while( ++Cfn <= asmFileCount && false == ctx.m_Finished) {
		bool result;

		result = OpenInputFile(&ctx, *np, false);
		if(true == result) {
			AssembleFile(&ctx, *np);
		}
		np++;
	}

	if(0 != GetOpenFileCount()) {
		internal((&ctx, "Error in filecount: %i\n", GetOpenFileCount()));
	}

	/* If no errors or a listing has been requested
		increase pass count go keep going */
	if(0 == ctx.m_ErrorCount || true == ctx.m_ListingFlags.m_OptEnabled) {
		ctx.m_Pass++;

		/* If errors have occured turn off the output file */
		if(0 != ctx.m_ErrorCount) {
			ctx.m_Misc.Oflag = false;
		}

		np = filelist;

		Reinitialize(&ctx);

		if(ctx.m_Misc.Oflag == true) {
			bool result;

			if(outputFilename) {
				result = OpenOutput(&ctx, outputFilename, false);
			} else if(NULL != filelist[0]) {
				result = OpenOutput(&ctx, filelist[0], true);
			} else {
				result = true;
			}

			if(false == result) {
				return 1;
			}
		}

		while( ++Cfn <= asmFileCount) {
			if(OpenInputFile(&ctx, *np, false) == true) {
				ctx.m_Finished = false;
				AssembleFile(&ctx, *np);
			}

			np++;
		}

		CloseOutput(&ctx);                 /* output closing record */

		if(true == ctx.m_Misc.Cflag) {
			PrintCycles(&ctx, ctx.m_CycleTotal);  /* if still counting cycles, then  print cycles counted */
		}

		if(ctx.m_SilentMode == false) {
			if (ctx.m_Misc.Sflag == 1) {
				DumpSymTable(&ctx);
			}

			if (true == ctx.m_ListingFlags.m_ListSymbols) {
				 DumpCrossRef();
			}
		}
	}

#ifdef _WIN32
	endTime = GetTickCount();
#endif

	if(ctx.m_SilentMode == false) {
		fprintf(stderr, "\n\n");

		if(ctx.m_Misc.OptFlag == true || ctx.m_Misc.OptShow == true) {
			fprintf(stderr, "Number of lines that can be optimized: %u\n", OptCount);
		}

#ifdef _WIN32
		fprintf(stderr, "%d lines assembled in %2.2f seconds\n", lineCount, (double)(endTime - startTime) / 1000);
#endif
		if(0 != ctx.m_ErrorCount) {
			fprintf(stderr, "Number of errors %d\n", ctx.m_ErrorCount);
		}

		if(0 != ctx.m_WarningCount) {
			fprintf(stderr, "Number of warnings %d\n", ctx.m_WarningCount);
		}
	}

	FwdRefDone();

	return(0 != ctx.m_ErrorCount ? ERR_GENERAL : ERR_SUCCESS);
}

