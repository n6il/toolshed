/*****************************************************************************
	pseudo.c	- Implementation of pseudo instructions

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

/*============================================================================
	TODO::

		endm	OS-9 end module


		change page to send commend to output_listing.c
		fix org to detect if it's allowed
		fix rmb and related to emit uninit data and have the output handler deal with it
		fix setdp to allow 'SETDP none' to disable the DP register
		fix export to detect if it's allowed
		fix equ to detect failure on evaluator
		update 'end' to set the entry point for modinit if it's not declared
		fix psect to support full OS-9 info 'psect modname,val,val,val,stack,entry'
		fix vsect to support 'vsect dp'

============================================================================*/


#include "as.h"
#include "proto.h"
#include "os9.h"
#include "label.h"
#include "input.h"


/*

   1
   2
   3  0000 34   40                     pshs    u
   4  0002 1F   43                     tfr     s,u
   5
   6  0004 32   e89c                   leas    -100,s
   7
   8  0007 1F   34                     tfr     u,s
   9  0009 35   c0                     puls    u,pc
  10


    
*/



/*============================================================================
	Set direct page


============================================================================*/
static void PseudoSetDP(EnvContext *ctx)
{
	int32	result;
	
	Evaluate(ctx, &result, EVAL_NORMAL, NULL);
	if(result > MAX_UBYTE) {
		warning(ctx, WARN_DPRANGE, "DP value cannot be larger than 8 bits - value truncated");
	}
	SetDPReg(lobyte(result));
}



/*============================================================================
	OS-9 Module generation ops


============================================================================*/
static void PseudoModule(EnvContext *ctx)
{
	bool		status;
	u_char		parity;
	int32		mod_size;
	int32		mod_nameoffset;
	int32		mod_typelang;
	int32		mod_attrev;
	int32		mod_entrypoint;
	int32		mod_uninitsize;
	static const char *errormsg = "Error encountered processing OS-9 module header declaration";

	if(OS9BIN != ctx->m_OutputType) {
		warning(ctx, WARN_OS9HEADER, "generating OS-9 module header. Program counter reset to 0");
	}

	if(1 == ctx->m_Pass) {
		int i;

		for(i = 0; i < 13; i++) {
			EmitOpDataByte(ctx, 0x00);
		}
		return;
	}
		
	/* reset the PC register */
	SetPCReg(0);
	SetOldPCReg(0);

	/* Process the module size */
	status = Evaluate(ctx, &mod_size, EVAL_NORMAL, NULL);
	if(false == status) {
		error(ctx, ERR_SYNTAX, errormsg);
		return;
	}

	/* Process the offset to the module name */
	status = Evaluate(ctx, &mod_nameoffset, EVAL_NORMAL, NULL);
	if(false == status) {
		error(ctx, ERR_SYNTAX, errormsg);
		return;
	}

	/* Process the type/lang */
	status = Evaluate(ctx, &mod_typelang, EVAL_NORMAL, NULL);
	if(false == status) {
		error(ctx, ERR_SYNTAX, errormsg);
		return;
	}

	/* Process the attribute and revision */
	status = Evaluate(ctx, &mod_attrev, EVAL_NORMAL, NULL);
	if(false == status) {
		error(ctx, ERR_SYNTAX, errormsg);
		return;
	}

	/* Process the entry point */
	status = Evaluate(ctx, &mod_entrypoint, EVAL_NORMAL, NULL);
	if(false == status) {
		error(ctx, ERR_SYNTAX, errormsg);
		return;
	}

	/* Process the size of uninitialized data */
	status = Evaluate(ctx, &mod_uninitsize, EVAL_NORMAL, NULL);
	if(false == status) {
		error(ctx, ERR_SYNTAX, errormsg);
		return;
	}

	mod_size += 3;	/* add 3 bytes to account for the module CRC */


	parity = os_parity(OS9MOD_SIG,
					   loword(mod_size),
					   loword(mod_nameoffset),
					   lobyte(mod_typelang),
					   lobyte(mod_attrev));
	
	EmitOpDataWord(ctx, OS9MOD_SIG);				/* Emit the OS-9 header signature */
	EmitOpDataWord(ctx, loword(mod_size));			/* Emit module size */
	EmitOpDataWord(ctx, loword(mod_nameoffset));	/* emit module name offset */
	EmitOpDataByte(ctx, lobyte(mod_typelang));		/* emit type/language */
	EmitOpDataByte(ctx, lobyte(mod_attrev));		/* emit attribute and revision */
	EmitOpDataByte(ctx, parity);					/* emit parity */
	EmitOpDataWord(ctx, loword(mod_entrypoint));	/* Emit execution offset */
	EmitOpDataWord(ctx, loword(mod_uninitsize));	/* Emit size of uninitialized data */
}


static void PseudoOS9(EnvContext *ctx)
{
	int32	result;

	/* FIXME
	if(OS9BIN != ctx->m_OutputType) {
		if(strictLevel > STRICT_NONE) {
			error(ctx, ERR_GENERAL, "'os9' directive only supported for OS-9 binaries");
			return;
		} else {
			warning(ctx, WARN_OS9SYSCALL, "generating OS-9 system call");
		}
	}
	*/

	ctx->m_Ptr = SkipConstWS(ctx->m_Ptr);	/* Move over IsWS spaces */
	Evaluate(ctx, &result, EVAL_NORMAL, NULL);
	if(0 != hibyte(result) && 0xff != hibyte(result)) {
		warning(ctx, WARN_OS9SYSCALL, "system call value too large - value truncated to '$%02X'", lobyte(result));
	}
	
	EmitOpCode2(ctx, OP_SWI, lobyte(result));
}


static void PseudoSegmentPSECT(EnvContext *ctx, const Mneumonic *op)
{
	if(OS9BIN != ctx->m_OutputType) {
		warning(ctx, WARN_PSECTUSAGE, "use of psect is only applicable to the creation of OS-9 modules");
	} else {
		BeginSegment(ctx, SEGMENT_CODE);
	}
}


static void PseudoSegmentVSECT(EnvContext *ctx, const Mneumonic *op)
{
	if(OS9BIN != ctx->m_OutputType) {
		warning(ctx, WARN_VSECTUSAGE, "use of vsect is only applicable to the creation of OS-9 modules");
	} else {
		BeginSegment(ctx, SEGMENT_DATA);
	}
}


static void PseudoEndSegment(EnvContext *ctx)
{
	if(OS9BIN != ctx->m_OutputType) {
		warning(ctx, WARN_ENDSECTUSAGE, "use of endsect is only applicable to the creation of OS-9 modules");
	} else {
		EndSegment(ctx);
	}
}





/*============================================================================



============================================================================*/
static void PseudoInclude(EnvContext *ctx, const Mneumonic *op)
{
	char buffer[MAX_BUFFERSIZE];
	const char *src;
	char *dst;
	char delim;


	src = ctx->m_Operand;
	dst = buffer;
	if('\'' == *src || '"' == *src) {
		delim = *(src++);
	} else {
		delim = 0x00;
	}

	while(*src) {
		if(0 != delim) {
			if(*src == delim) {
				break;
			}
		} else {
			if(true == IsWS(*src)) {
				break;
			}
		}
		*(dst++) = *(src++);
	}
	*dst = 0;

	dst= buffer;
	while(*dst) {
		if('/' == *dst) {
			if((3 == strlen(dst) || ':' == dst[4]) && isdigit(dst[5])) {
				*dst = '.';
			}
		}

		if(':' == *dst && isdigit(dst[1]) && EOS == dst[2]) {
			*dst = EOS;
			break;
		}

		dst++;
	}

	OpenInputFile(ctx, buffer, false);
}


static void PseudoRaw(EnvContext *ctx)
{
	int j;

	/* FIXME - add checks for quoted paths and comments */
	if(OpenInputFile(ctx, ctx->m_Operand, false) == true) {
		int fill;
		
		fill = GetCurrentInputFileSize();

		if(fill < 0) {
			error(ctx, ERR_GENERAL, "An unknown error occured while obtaining the length of %s", ctx->m_Operand);
			return;
		}
		
		if(fill > MAX_WORD) {
			error(ctx, ERR_GENERAL, "RAW file is to long, must be smaller than %dk", MAX_WORD / 1024);
		} else {
			if(ctx->m_Pass == 1) {
				/* FIXME - update for handling generation of obj files */
				BumpPCReg(loword(fill));
			} else {
				for(j = 0; j < fill; j++) {
					EmitDataByte(ctx, lobyte(ReadInputByte()));
				}
			}
		}
		CloseInputFile(ctx);
	} else {
		error(ctx, ERR_GENERAL, "unable to open '%s'", ctx->m_Operand);
	}
}


/*============================================================================


============================================================================*/
static void PseudoSegmentCode(EnvContext *ctx, const Mneumonic *op)
{
	if(OBJBIN != ctx->m_OutputType) {
		warning(ctx, WARN_SEGMENTUSAGE, "use of '%s' is only applicable to the creation of .obj files", op->mnemonic);
	} else {
		SetSegment(ctx, SEGMENT_CODE);
	}
}


static void PseudoSegmentData(EnvContext *ctx, const Mneumonic *op)
{
	if(OBJBIN != ctx->m_OutputType) {
		warning(ctx, WARN_SEGMENTUSAGE, "use of '%s' is only applicable to the creation of .obj files", op->mnemonic);
	} else {
		SetSegment(ctx, SEGMENT_DATA);
	}
}


static void PseudoSegmentBSS(EnvContext *ctx, const Mneumonic *op)
{
	if(OBJBIN != ctx->m_OutputType) {
		warning(ctx, WARN_SEGMENTUSAGE, "use of '%s' is only applicable to the creation of .obj files", op->mnemonic);
	} else {
		SetSegment(ctx, SEGMENT_BSS);
	}
}




/*============================================================================


============================================================================*/
static void PseudoMacro(EnvContext *ctx)
{
	if(IsProcessingMacro() == true) {
		error(ctx, ERR_GENERAL, "cascading macro declarations not allowed");
	} else {
		CreateMacro(ctx, ctx->m_Label, ctx->m_Operand);	/* create the macro FIXME */
	}
}




/*============================================================================


============================================================================*/
static void PseudoALIGN(EnvContext *ctx)
{
	int32	result;

	if(false == Evaluate(ctx, &result, EVAL_NORMAL, NULL)) {
		return;
	}

	if(result < 1 || result > 8192) {
		error(ctx, ERR_GENERAL, "alignment value must be greater than zero and less than 8192");
		return;
	}

	result = (result - ((GetPCReg() + result) % result));
	if(MOTBIN == ctx->m_OutputType) {
		EmitUninitData(ctx, loword(result));
	} else {
		while(result > 0) {
			EmitOpDataByte(ctx, 0);
			result--;
		}
	}
}


static void PseudoEVEN(EnvContext *ctx)
{
	if(GetPCReg() & 0x01) {
		EmitOpDataByte(ctx, 0);
	}
}


static void PseudoODD(EnvContext *ctx)
{
	if(!(GetPCReg() & 0x01)) {
		EmitOpDataByte(ctx, 0);
	}
}




/*============================================================================


============================================================================*/
static void PseudoOrigin(EnvContext *ctx)
{
	int32 result;
	bool status;
	int modMMUPage;

	modMMUPage = -1;

	if(MODBIN == ctx->m_OutputType && true == ctx->m_OriginSet) {
		error(ctx, ERR_GENERAL, "modules can only have one code section (ORG statement)");
		return;
	}

	status = Evaluate(ctx, &result, EVAL_NORMAL, NULL);
	if(true == status) {

		/*
			If there is a command after the first address it should be treated as
			an MMU page with the exec address following the comma.
		*/
		if(',' == *ctx->m_Ptr && MODBIN == ctx->m_OutputType) {
			if(result < 0 || result > 0xff) {
				error(ctx, ERR_GENERAL, "module MMU page out of range for origin statement");
			}
			modMMUPage = (u_char)result;
			ctx->m_Ptr++;

			status = Evaluate(ctx, &result, EVAL_NORMAL, NULL);
		}
	}

	if(0 != hiword(result) && 0xffff != hiword(result)) {
		warning(ctx, WARN_VALUE_TRUNCATED, "origin address out of range - value truncated to $%04x", loword(result));
	}

	if(true == status) {
		SetPCReg(loword(result));
		SetOldPCReg(loword(result));
		FlushOutput(ctx);     /* flush out bytes */

		if(false == ctx->m_OriginSet) {
			ctx->m_OrgBase = loword(result);
			ctx->m_OriginSet = true;
		}

	}

	SetExportAddress(modMMUPage, (u_int16)result);
}




/*============================================================================


============================================================================*/
static void PseudoOption(EnvContext *ctx)
{
	char buffer[MAX_BUFFERSIZE];
	const char *ptr;


	ctx->m_ListingFlags.P_force = false;

	strcpy(buffer, ctx->m_Operand);
	strlwr(buffer);

	if(true == ctx->m_ListingFlags.m_OptEnabled) {
		Cpflag |= 2;	/* why is this here???? */
	}


	ptr = ctx->m_Ptr;
	do {
		ptr = SkipConstWS(ptr);

		if(head(ptr, "6809")) {
			ptr += 4;
			ctx->m_CPUType = CPU_6809;
		}

		else if(head(ptr, "6309")) {
			ptr += 4;
			ctx->m_CPUType = CPU_6309;
		}

		else if(head(ptr, "prec")) {
			ptr += 4;
			ctx->m_Compat.m_UsePrecedence = true;
		}

		else if(head(ptr, "noprec")) {
			ptr += 6;
			ctx->m_Compat.m_UsePrecedence = false;
		}


		else if(head(ptr,"l") ) {
			ptr++;
			if(true == ctx->m_ListingFlags.m_CmdEnabled) {
				ctx->m_ListingFlags.m_OptEnabled = true;
			}
		}

		else if(head(ptr,"nol")) {
			ptr += 3;
			if(true == ctx->m_ListingFlags.m_CmdEnabled) {
				ctx->m_ListingFlags.m_OptEnabled = false;
			}
		}

		else if(head(ptr,"c")) {
			ptr++;
			ctx->m_Misc.Cflag = true;
			ctx->m_CycleTotal = 0;
		}
		
		else if(head(ptr,"noc")) {
			ptr += 3;
			ctx->m_Misc.Cflag = false;
			Cpflag |= 1;
		}
		
		else if(head(ptr,"contc")) {
			ptr += 5;
			ctx->m_Misc.Cflag = true;
		}

		else if(head(ptr,"s")) {
			ptr++;
			ctx->m_Misc.Sflag = true;
		}

		else if(head(ptr,"cre")) {
			ptr += 3;
			ctx->m_ListingFlags.m_ListSymbols = true;
		}

		else {
			error(ctx, ERR_SYNTAX, "unrecognized or missing OPT");
		}

	} while( *ptr++ == ',' );
}




/*============================================================================


============================================================================*/
static void PseudoPage(EnvContext *ctx)
{
	ctx->m_ListingFlags.P_force = false;
	ctx->m_Misc.N_page = true;

	if(ctx->m_Pass == 2 && true == ctx->m_ListingFlags.m_OptEnabled) {
		printf("\f");
		printf("%-40s", Argv[Cfn]);
		printf("     ");
		printf("page %3d\n", ctx->m_ListingFlags.m_PageNumber++);
	}
}




/*============================================================================


============================================================================*/
static bool GetImportExportLabel(EnvContext *ctx, const Mneumonic *op, char *out, int max)
{
	char *dst;

	// Check for local labels
	if(true == HasLocalLabelChar(ctx, ctx->m_Ptr)) {
		error(ctx, ERR_INVALID_LABEL, "Local labels not allowed in %s directive", op->mnemonic);
		return false;
	}

	// Check for import/export label start
	if(false == IsExportLabelStart(ctx, *ctx->m_Ptr)) {
		error(ctx, ERR_INVALID_LABEL, "Invalid label for %s directive", op->mnemonic);
		return false;
	}

	// Copy the label
	dst = out;
	while(true == IsExportLabelChar(ctx, *ctx->m_Ptr) && max > 0) {
		*(dst++) = *(ctx->m_Ptr++);
	}
	*dst = EOS;

	if(EOS != *ctx->m_Ptr && false == IsWS(*ctx->m_Ptr)) {
		if(true == IsExportLabelChar(ctx, *ctx->m_Ptr)) {
			error(ctx, ERR_LABEL_TOO_LONG, "%s label too long", op->mnemonic);
		} else {
			error(ctx, ERR_INVALID_LABEL, "Invalid character in %s label", op->mnemonic);
		}
		return false;
	}

	return true;
}


static void PseudoExport(EnvContext *ctx, const Mneumonic *op)
{
	if(1 == ctx->m_Pass) {
		char buffer[MAX_LABELSIZE + 1];
		bool result;

		result = GetImportExportLabel(ctx, op, buffer, MAX_LABELSIZE);
		if(false == result) {
			return;
		}

		AddExport(ctx, buffer);
	}
}


static void PseudoImport(EnvContext *ctx, const Mneumonic *op)
{
	if(1 == ctx->m_Pass) {
		char buffer[MAX_LABELSIZE + 1];
		bool result;

		result = GetImportExportLabel(ctx, op, buffer, MAX_LABELSIZE);
		if(false == result) {
			return;
		}

		AddSymbol(ctx, buffer, 0, SYM_IMPORTED, NULL, 0);
	}
}




/*============================================================================


============================================================================*/
static void PseudoEnd(EnvContext *ctx)
{
	int32 result;

	ctx->m_Finished = true;

	if(EOS != *ctx->m_Ptr && false == IsWS(*ctx->m_Ptr)) {
		Evaluate(ctx, &result, EVAL_NORMAL, NULL);

		if(result) {
			if((unsigned int)result > (unsigned int)0xFFFF) {
				warning(ctx, WARN_VALUE_TRUNCATED, "entry point too large - value will be truncated to $04x", loword(result));
			}
			if( ctx->m_HasEntryPoint ) {
				if(result == ctx->m_EntryPoint) {
					warning(ctx, WARN_ENTRYPOINT, "entry point already specified");
				}

				else {
					warning(ctx, WARN_ENTRYPOINT, "entry point redefined");
				}
			}
			else {
				ctx->m_EntryPoint = (u_int16)result;
				ctx->m_HasEntryPoint = true;
			}
		} else {
			warning(ctx, WARN_ENTRYPOINT, "comments following END should start with semicolon");
		}
	}
}




/*============================================================================


============================================================================*/
static void PseudoEQU(EnvContext *ctx, const Mneumonic *op)
{
	int32 result;

	if(*ctx->m_Label == EOS) {
		error(ctx, ERR_SYNTAX, "'%s' requires label", op->mnemonic);
		return;
	}
	Evaluate(ctx, &result, EVAL_NORMAL, NULL);
	AddSymbol(ctx, ctx->m_Label, (int32)result, SYM_VALUE, 0, 0);
	
	
	/*
		Kludge - Set the old pc address so the EQU value is printed in the
		listing as the PC address.
	*/
	SetOldPCReg(loword(result));
}






/*============================================================================


============================================================================*/
static void PseudoNAMESPACE(EnvContext *ctx)
{
	char ns[MAX_LABELSIZE + 1];
	char *dst;
	const char *src;
	int size;

	if(true == IsInNamespace()) {
		error(ctx, ERR_GENERAL, "nested namespaces not allowed");
		return;
	}

	if(false == IsLabelStart(ctx, *ctx->m_Label)) {
		error(ctx, ERR_SYNTAX, "invalid label for namespace");
		return;
	}

	src = ctx->m_Label;
	dst = ns;
	size = 0;
	while(true == IsLabelChar(ctx, *src) && size < MAX_LABELSIZE) {
		*(dst++) = *(src++);
		size++;
	}

	if(size == MAX_LABELSIZE) {
		error(ctx, ERR_GENERAL, "namespace is too long");
		return;
	}

	if(0 != *src) {
		error(ctx, ERR_SYNTAX, "invalid character in namespace label");
		return;
	}

	*dst = 0;

	SetNamespace(ctx, ns);
}


static void PseudoENDNAMESPACE(EnvContext *ctx)
{
	ClearNamespace(ctx);
}




/*============================================================================


============================================================================*/
static void PseudoZMB(EnvContext *ctx, const Mneumonic *op, int multiplier)
{
	int32	result;

	if(Evaluate(ctx, &result, EVAL_NORMAL, NULL)) {
		result *= multiplier;

		while(result--) {
			EmitDataByte(ctx, 0);
		}
	}
}


static void PseudoFill(EnvContext *ctx, const Mneumonic *op)
{
	int32	result;
	int i;

	Evaluate(ctx, &result, EVAL_NORMAL, NULL);
	i = result;
	if(*ctx->m_Ptr++ != ',') {
		error(ctx, ERR_SYNTAX, "missing comma");
	}

	else {
		bool status;

		ctx->m_Ptr = SkipConstWS(ctx->m_Ptr);
		status = Evaluate(ctx, &result, EVAL_NORMAL, NULL);
		if(false == status || result < 0 || result > MAX_WORD) {
			error(ctx, ERR_GENERAL, "Invalid value specified for '%s'", op->mnemonic);
			return;
		}

		while(result--) {
			EmitDataByte(ctx, lobyte(i));
		}
	}
}


static void PseudoFCB(EnvContext *ctx, const Mneumonic *op)
{
	while(true) {
		bool	status;
		int32	result;
		bool	forceFDB;

		if('>' == *ctx->m_Ptr) {
			forceFDB = true;
			ctx->m_Ptr++;
		} else {
			forceFDB = false;
		}


		status = Evaluate(ctx, &result, EVAL_INDEXED_FCB, NULL);
		if(false == status) {
			break;
		}

		if('[' == *ctx->m_Ptr) {
			u_char	values[MAX_FCB_REPEATS];
			int32	repeatCount;
			int		count;

			/* Check the count value */
			if(result < 0) {
				error(ctx, ERR_SYNTAX, "Repeating values must have a count greater than 0");
				break;
			}


			repeatCount = result;
			count = 0;

			/* Collect the data */
			do {
				ctx->m_Ptr++;
				status = Evaluate(ctx, &result, EVAL_NORMAL, NULL);
				if(false == status) {
					break;
				}
				values[count++] = (u_char)result;
			} while(',' == *ctx->m_Ptr);

			if(']' == *ctx->m_Ptr) {
				ctx->m_Ptr++;
			} else {
				error(ctx, ERR_SYNTAX, "Expected closing bracket on repeating value");
				break;
			}

			while(repeatCount > 0) {
				int i;

				for(i = 0; i < count; i++) {
					if(false == forceFDB) {
						EmitDataByte(ctx, values[i]);
					} else {
						EmitDataWord(ctx, values[i]);
					}
				}

				repeatCount--;
			}

		} else {
			if(false == forceFDB) {
				if(0 != hibyte(result) && 0xff != hibyte(result)) {
					warning(ctx, WARN_VALUE_TRUNCATED, "value truncated from %02x to $%02x", result, lobyte(result));
					result = lobyte(result);
				}
				EmitDataByte(ctx, lobyte(result));
			} else {
				if(0 != hiword(result) && 0xffff != hiword(result)) {
					warning(ctx, WARN_VALUE_TRUNCATED, "value truncated from %04x to $%04x", result, loword(result));
					result = lobyte(result);
				}
				EmitDataWord(ctx, loword(result));
			}
		}

		if(',' != *ctx->m_Ptr) {
			break;
		}

		ctx->m_Ptr++;
	}
}


static void PseudoFDB(EnvContext *ctx, const Mneumonic *op)
{
	int32 result;

	do {
		ctx->m_Ptr = SkipConstWS(ctx->m_Ptr);
		Evaluate(ctx, &result, EVAL_NORMAL, NULL);

		if(0 != hiword(result) && 0xffff != hiword(result)) {
			warning(ctx, WARN_VALUE_TRUNCATED, "value truncated to %04x", loword(result));
		}
		EmitDataWord(ctx, loword(result));
	} while( *ctx->m_Ptr++ == ',' );
}


static void PseudoFQB(EnvContext *ctx, const Mneumonic *op)
{
	int32	result;

	do {
		ctx->m_Ptr = SkipConstWS(ctx->m_Ptr);
		Evaluate(ctx, &result, EVAL_NORMAL, NULL);
		EmitDataLong(ctx, result);
	} while( *ctx->m_Ptr++ == ',' );
}


static void PseudoFCCType(EnvContext *ctx,
						  const Mneumonic *op,
						  u_char lastCharOrVal,
						  int trailingCount,
						  const u_char *trailingBytes)
{
	char	fccdelim;
	char	fcschar;

	if(EOS == *ctx->m_Ptr || true == IsWS(*ctx->m_Ptr)) {
		error(ctx, ERR_SYNTAX, "invalid character specified as delimiter for %s", op->mnemonic);
		return;
	}

	fccdelim = *ctx->m_Ptr++;

	while(*ctx->m_Ptr != EOS && *ctx->m_Ptr != fccdelim) {
		fcschar = *ctx->m_Ptr++;
		if(*ctx->m_Ptr == fccdelim || 0 == *ctx->m_Ptr) {
			fcschar |= lastCharOrVal;
		}
		EmitDataByte(ctx, fcschar);
	}

	if(*ctx->m_Ptr != fccdelim) {
		error(ctx, ERR_SYNTAX, "missing closing delimiter for %s", op->mnemonic);
		return;

	}

	ctx->m_Ptr++;

	if(',' == *ctx->m_Ptr) {
		ctx->m_Ptr++;
		PseudoFCB(ctx, op);
	}

	if(!(true == IsWS(*ctx->m_Ptr) || EOS == *ctx->m_Ptr)) {
		error(ctx, ERR_SYNTAX, "Unhandled data extension");
	}

}

static void PseudoFCC(EnvContext *ctx, const Mneumonic *op)
{
	PseudoFCCType(ctx, op, 0x00, 0, NULL);
}


static void PseudoFCS(EnvContext *ctx, const Mneumonic *op)
{
	PseudoFCCType(ctx, op, 0x80, 0, NULL);
}

static void PseudoFCZ(EnvContext *ctx, const Mneumonic *op)
{
	static const u_char trails[1] = { 0x00 };
	PseudoFCCType(ctx, op, 0x00, 1, trails);
}


static void PseudoFCR(EnvContext *ctx, const Mneumonic *op)
{
	static const u_char trails[2] = { '\n', 0x00 };
	PseudoFCCType(ctx, op, 0, 2, trails);
}




/*============================================================================


============================================================================*/
static void PseudoRMB(EnvContext *ctx, const Mneumonic *op, int multiplier)
{
	int32	result;

	if(MODBIN == ctx->m_OutputType) {
		warning(ctx, WARN_UNINITUSAGE, "creating module. 'zmb' used instead of '%s'", op->mnemonic);
		PseudoZMB(ctx, op, multiplier);
		return;
	}

	if(Evaluate(ctx, &result, EVAL_NORMAL, NULL)) {
		result *= multiplier;
		EmitUninitData(ctx, loword(result));
	}
}




/*============================================================================


============================================================================*/

static void PseudoStruct(EnvContext *ctx)
{
	if(ctx->m_Pass == 1) {
		StructCreate(ctx, ctx->m_Label);
	} else {
		laststructfound = StructLookup(ctx->m_Label);
		ASSERTX(NULL != laststructfound);
		SetOldPCReg(loword(laststructfound->size));
	}
	
	instruct = true;
}


static void PseudoStructDec(EnvContext *ctx)
{		
	int32 result;

	ctx->m_Ptr = SkipConstWS(ctx->m_Ptr);
	if(0 != *ctx->m_Ptr && false == IsCommentChar(ctx, *ctx->m_Ptr)) {
		bool status;
		
		status = Evaluate(ctx, &result, EVAL_NORMAL, NULL);
		if(false == status) {
			error(ctx, ERR_SYNTAX, "invalid expression for type declaration");
		}

		else if(result < 1) {
			result = 1;
			error(ctx, ERR_GENERAL, "cannot allocate an array of constant size %d", result);
		}
	} else {
		result = 1;
	}


	AddSymbol(ctx, ctx->m_Label, GetPCReg(), SYM_STRUCTDATA, laststructfound, result);
	result *= laststructfound->size;

	if(1 == ctx->m_Pass) {
		BumpPCReg(loword(result));
	} else {
		while(result--) {
			EmitDataByte(ctx, 0);
		}
	}
}





void PseudoFail(EnvContext *ctx)
{
	char buffer[MAX_BUFFERSIZE + 1];

	if(EOS != *ctx->m_Ptr) {
		char *dst;
		char faildelim;

		faildelim = *(ctx->m_Ptr++);
		dst = buffer;

		while(EOS != *ctx->m_Ptr && faildelim != *ctx->m_Ptr) {
			*(dst++) = *(ctx->m_Ptr++);
		}
		*dst = EOS;

	} else {
		buffer[0] = EOS;
	}

	if(EOS == buffer[0]) {
		fatal(ctx, "assembly failed with no error message");
	} else {
		fatal(ctx, "assembly failed: %s", buffer);
	}
}




/*============================================================================
	Pseudo op processing

	ProcPseudo is always called regardless of the current state of the
	assembler. This is required as pseudos related to conditionals, macros,
	structures, unions, and namespaces must all be processed even if the
	current state would otherwise preclude the op from being handled. It is
	up to ProcPseudo to make the determination of whether the op should be
	processed or not.

============================================================================*/
void ProcPseudo(EnvContext *ctx, const Mneumonic *op)
{
	bool		condition;
	int			condCount;
	PSEUDO_OP	conditionOp;

	condition = GetConditionalState(ctx);
	condCount = GetConditionalCount(ctx);
	if(NO_CONDITION != condCount) {
		conditionOp = GetConditionalOp(ctx);
	} else {
		conditionOp = NULL_OP;
	}

	
	/*
		ENDM must be process here since all other lines should simply be added
		to the macro definition.

		**NOTE** Conditional statements are handled AFTER the macro in order to
		allow changes in environment when processing conditionals within a macro.
		This allows the code within a macro's conditional to be assembled in early
		uses and ommited in later stages.
	*/
	if(ENDM == op->opcode) {
		EndMacro(ctx);
		return;
	}

	
	/* If processing a macro, go no further */
	if(IsProcessingMacro() == true) {
		return;
	}
	
	/*
	process pseudo instructions that are allowed during a conditional assembly
	*/
	if(ENDP == op->opcode) {
		if(NO_CONDITION == condCount || (IFP1 != conditionOp && IFP2 != conditionOp)) {
			error(ctx, ERR_MISMATCHED_COND, "ENDP without IFP1 or IFP2 conditional");
		}

		else {
			PopConditionalState(ctx);
		}

		return;
	}

	if(ENDC == op->opcode) {
		if(NO_CONDITION == condCount) {
			error(ctx, ERR_MISMATCHED_COND, "ENDC without COND conditional");
			return;
		}

		if(COND != conditionOp && true == ctx->m_Compat.m_Warn) {
			warning(ctx, WARN_MISMATCHED_COND, "ENDC required matching COND statement");
		}

		PopConditionalState(ctx);

		return;
	}

	if(ENDIF == op->opcode) {
		if(NO_CONDITION == condCount) {
			error(ctx, ERR_MISMATCHED_COND, "ENDIF statement without matching conditional");
			return;
		}

		/* FIXME
		if(strictLevel > STRICT_NONE) {
			if(IFP1 == conditionalOP) {
				error(ctx, ERR_MISMATCHED_COND, "'ifp1' requires 'endp' not 'endif'");
			}
			
			else if(IFP2 == conditionalOP) {
				error(ctx, ERR_MISMATCHED_COND, "'ifp2' requires 'endp' not 'endif'");
			}

			else if(COND == conditionalOP) {
				error(ctx, ERR_MISMATCHED_COND, "'cond' requires 'endc' not 'endif'");
			}
		*/

		PopConditionalState(ctx);
		
		return;
	}


	/* Handle ELSE statements */
	if(ELSE == op->opcode) {
		if(NO_CONDITION == condCount) {
			error(ctx, ERR_MISMATCHED_COND, "'else' used without matching conditional statement");
			return;
		}

		ToggleConditionalState(ctx);

		return;
	}


	/* If the current condition is false, don't do anything */
	if(condition == false) {
		return;
	}

	if(0 != ctx->m_Label[0] && EQU != op->opcode && STRUCT != op->opcode && STRUCTDEC!= op->opcode && NAMESPACE != op->opcode) {
		AddSymbol(ctx, ctx->m_Label, GetPCReg(), SYM_ADDRESS, NULL, 0);
	}

	ctx->m_ListingFlags.P_force = true;

	switch(op->opcode) {
	case PRINTDP:
		note(ctx, "DP Value is %02X\n", GetDPReg());
		break;

	/* ignored psuedo ops */
	case NULL_OP:
		ctx->m_ListingFlags.P_force = false;
		break;

	case SETDP:
		PseudoSetDP(ctx);	/* Set the direct page register value */
		break;

	case PUSHDP:
		PushDPReg(ctx);		/* Save the current Direct Page register value */
		break;

	case POPDP:
		PopDPReg(ctx);			/* Restore the previous direct page register value */
		break;

	case MACRO:
		PseudoMacro(ctx);	/* handle macro declarations */
		break;

	case RAW:
		PseudoRaw(ctx);	/* Import a raw file into the assembler stream */
		break;

	case LIB:
		PseudoInclude(ctx, op);	/* Include file */
		break;

	case IFDEF:				/* Is symbol is defined */
	case IFNDEF:			/* If symbol is not defined */
	case IFP1:				/* If on pass 1 */
	case IFP2:				/* If on pass 2 */
	case IFNE:				/* Not equal */
	case IFLT:				/* Less than */
	case IFLE:				/* Less than or equal */
	case IFGT:				/* Greater than */
	case IFGE:				/* Creater than or equal */
	case IFEQ:				/* if value is not 0 */
	case IF:				/* if value is not 0, assemble */
	case IFN:				/* if value is is 0, assemble */
	case COND:				/* true conditional */
		PushConditionalState(ctx, op->mnemonic, (PSEUDO_OP)op->opcode);
		/*PseudoConditional(ctx, op);*/
		break;


	case RMB:
		PseudoRMB(ctx, op, 1);	/* Reserve bytes */
		break;

	case RMD:
		PseudoRMB(ctx, op, 2);	/* Reserve words */
		break;

	case RMQ:
		PseudoRMB(ctx, op, 4);	/* Reserve longs */
		break;
	
	case ZMB:
		PseudoZMB(ctx, op, 1);	/* Reserve bytes and fill with 0 */
		break;

	case ZMD:
		PseudoZMB(ctx, op, 2);	/* Reserve words and fill with 0 */
		break;

	case ZMQ:
		PseudoZMB(ctx, op, 4);	/* Reserve logns and fill with 0 */
		break;

	case FILL:
		PseudoFill(ctx, op);	/* fill memory with constant */
		break;

	case FCB:
		PseudoFCB(ctx, op);	/* form character byte(s) */
		break;

	case FDB:
		PseudoFDB(ctx, op);	/* form double byte(s) */
		break;

	case FQB:
		PseudoFQB(ctx, op);	/* form quad byte(s) */
		break;

	case FCS:
		PseudoFCS(ctx, op);	/* Form command string */
		break;

	case FCC:
		PseudoFCC(ctx, op);	/* form constant characters */
		break;

	case FCZ:
	case FCN:
		PseudoFCZ(ctx, op);	/* Form null terminated string */
		break;

	case FCR:
		PseudoFCR(ctx, op);	/* Form cr/null terminated string */
		break;

	case ORG:
		PseudoOrigin(ctx);		/* origin */
		break;

	case EQU:
		PseudoEQU(ctx, op);	/* equate */
		break;

	case OPT:
		PseudoOption(ctx);		/* assembler option */
		break;

	case PAGE:
		PseudoPage(ctx);			/* go to a new page */
		break;

	case END:
		PseudoEnd(ctx);		/* END directive */
		break;

	case STRUCT:
		PseudoStruct(ctx);		/* Structure declaration */
		break;

	case STRUCTDEC:
		PseudoStructDec(ctx);	/* ctx->m_Finished of structure declaration */
		break;

	case ALIGN:
		PseudoALIGN(ctx);		/* Align PC on any value */
		break;

	case ALIGN_EVEN:
		PseudoEVEN(ctx);		/* Align PC on even boundry */
		break;

	case ALIGN_ODD:
		PseudoODD(ctx);			/* Align PC on odd boundry */
		break;

	case NAMESPACE:
		PseudoNAMESPACE(ctx);	/* Begin namespace */
		break;

	case ENDNS:
		PseudoENDNAMESPACE(ctx);	/* ctx->m_Finished namespace */
		break;

	case MOD:
		PseudoModule(ctx);		/* Generate an OS-9 module header */
		break;

	case OS9:
		PseudoOS9(ctx);		/* Generate an OS-9 system call */
		break;

	case VSECT:
		PseudoSegmentVSECT(ctx, op);
		break;

	case PSECT:
		PseudoSegmentPSECT(ctx, op);	/* Beginning of segment */
		break;

	case ENDSECT:
		PseudoEndSegment(ctx);		/* ctx->m_Finished of segment */
		break;

	case EXPORT:
		PseudoExport(ctx, op);	/* Export a symbol */
		break;

	case IMPORT:
		PseudoImport(ctx, op);	/* Export a symbol */
		break;

	case SEGCODE:
		PseudoSegmentCode(ctx, op);
		break;

	case SEGDATA:
		PseudoSegmentData(ctx, op);
		break;

	case SEGBSS:
		PseudoSegmentBSS(ctx, op);
		break;

	case FAIL:
		PseudoFail(ctx);
		break;

	case TITLE:
		strncpy(ctx->m_ListingFlags.m_Title, ctx->m_Operand, MAX_TITLESIZE - 1);
		ctx->m_ListingFlags.m_Title[MAX_TITLESIZE - 1] = 0;
		break;

	case NAME:
		break;


	case UNION:
	case ENDUNION:
		error(ctx, ERR_GENERAL, "invalid use of '%s'. Unions can only be created within structures", op->mnemonic);
		break;

	case REORG:	/* FIXME - not sure if this is the correct behavour */
		SetPCReg(loword(ctx->m_OrgBase));
		SetOldPCReg(loword(ctx->m_OrgBase));
		FlushOutput(ctx);     /* flush out bytes */
		break;

	case LIST:
		if(true == ctx->m_ListingFlags.m_CmdEnabled) {
			ctx->m_ListingFlags.m_OptEnabled = true;
		}
		break;

	case NLST:
		if(true == ctx->m_ListingFlags.m_CmdEnabled) {
			ctx->m_ListingFlags.m_OptEnabled = false;
		}
		break;

	case CLRD_6809:
		EmitOpCodeEx(ctx, 0x4f);
		EmitOpCodeEx(ctx, 0x5f);
		break;
		

	case OP_DEX:
		EmitOpCodeEx(ctx, 0x30);
		EmitOpPostByte(ctx, 0x1f);
		break;

	case OP_INX:
		EmitOpCodeEx(ctx, 0x30);
		EmitOpPostByte(ctx, 0x01);
		break;


	default:
		internal((ctx, "pseudo operation '%s' is unhandled by assembler", op->mnemonic));
		break;
	}

}
