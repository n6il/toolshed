/*****************************************************************************
	eval.c	- Expression evaluation

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
       eval --- evaluate expression
 
       an expression is constructed like this:

	   operator ::=	'+' |
					'-' |
					'*' |
					'/' |
					'|' |
					'&' |
					'%' |
					'^' |
					'=' |
					'<' |
					'>' |
					'<>' |
					'<=' |
					'>=' |
					'<<' |
					'>>' |
					'&&' |
					'||'
 
       expr ::=  <term> | <expr> <operator> <term> | "(" <expr> ")"

       term ::=  <symbol> | '*' | <constant>
 
       symbol ::=  [ "@" ] "a..z,_" { "0..9,a..z,_,$,@" } [ "." <symbol> ]
 
       constant ::= <hex_constant> |
                    <binary_constant> |
                    <octal_constant> |
                    <decimal_constant>
                    ascii constant ;
 
       hex_constant ::= "$" "0..9,a..f" { "0..9,a..f" }
 
       octal_constant ::= "!" "0..7" { "0..7" }
 
       binary_constant ::= "%" "0..1" { "0..1" }
 
       decimal_constant ::= "0..9" { "0..9" }
 
       ascii_constant ::= '<printing char>
 
*/
#include "as.h"
#include "proto.h"
#include "label.h"


typedef enum _OPERATOR {
	OP_NONE,
	OP_OROR,
	OP_ANDAND,
	OP_EQ,
	OP_NE,
	OP_LT,
	OP_GT,
	OP_LTE,
	OP_GTE,
	OP_OR,
	OP_EOR,
	OP_AND,
	OP_SHIFTL,
	OP_SHIFTR,
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_MOD,
} OPERATOR;


static bool EvaluateSub(EnvContext *ctx, int32 *retResult, EVAL_TYPE evalType, Symbol **retSym, int level);
static bool GetEvalTerm(EnvContext *ctx, long *retResult, EVAL_TYPE evalType, Symbol **retSym, int level);

	
static OPERATOR GetOperator(EnvContext *ctx, bool advance)
{
	OPERATOR op;
	int size;

	/* The default size of an operator is 1. */
	size = 1;

	switch(*ctx->m_Ptr) {
	case '+':
		op = OP_ADD;
		break;

	case '-':
		op = OP_SUB;
		break;

	case '*':
		op = OP_MUL;
		break;

	case '/':
		op = OP_DIV;
		break;

	case '%':
		op = OP_MOD;
		break;

	case '&':
		if('&' == ctx->m_Ptr[1]) {
			size = 2;
			op = OP_ANDAND;
		} else {
			op = OP_AND;
		}
		break;

	case '|':
		if('|' == ctx->m_Ptr[1]) {
			size = 2;
			op = OP_OROR;
		} else {
			op = OP_OR;
		}
		break;

	case '^':
		op = OP_EOR;
		break;

	case '=':
		op = OP_EQ;
		break;

	case '<':
		if('>' == ctx->m_Ptr[1]) {
			size = 2;
			op = OP_NE;
		}

		else if('=' == ctx->m_Ptr[1]) {
			size = 2;
			op = OP_LTE;
		}

		else if('<' == ctx->m_Ptr[1]) {
			size = 2;
			op = OP_SHIFTL;
		}

		else {
			op = OP_LT;
		}
		break;

	case '>':
		if('=' == ctx->m_Ptr[1]) {
			size = 2;
			op = OP_GTE;
		}

		else if('>' == ctx->m_Ptr[1]) {
			size = 2;
			op = OP_SHIFTR;
		}

		else {
			op = OP_GT;
		}
		break;

	default:
		op = OP_NONE;
		size = 0;
	}

	if(true == advance) {
		ctx->m_Ptr += size;
	}


	return op;
}


/*
	structures can be:
	object.element
	object.element.element
	object[index]
	object[index].element
	object[index].element.element
	object[index].element[index]
	object[index].element[index].element

	At this point:
		hold = the structure name
		sym->astruct = sym to the structure data
		operand = current ctx sym right after the named object
*/
static bool GetStructEvalTerm(EnvContext *ctx,
							  Struct *astruct,
							  int elmcount,
							  long *retResult,
							  EVAL_TYPE evalType,
							  bool checkbounds,
							  Symbol **retSym,
							  int level)
{
	char	hold[MAX_BUFFERSIZE];
	char	*tmp;
	long	terminal;
	int		offset;
	bool	hasError;


	hasError = false;

	/* Reset symbol offset to 0 */
	offset = 0;
	
	if(*ctx->m_Ptr == '[') {
		ctx->m_Ptr++;

		hasError = GetEvalTerm(ctx, &terminal, evalType, retSym, level);
		if(true == hasError) {
			*retResult = 0;
			return true;
		}

		if(*ctx->m_Ptr != ']') {
			error(ctx, ERR_SYNTAX, "subscript missing ']'");
		} else {
			ctx->m_Ptr++;

			if(true == checkbounds && terminal >= elmcount) {
				error(ctx, ERR_SUBSCRIPT_OUT_OF_RANGE, "subscript out of bounds accessing array element");
				return true;
			}
			offset += (astruct->size * terminal);
		}
	}

	/* Process sub elements */
	while('.' == *ctx->m_Ptr) {
		Element *el;
		int index;

		/* Skip period */
		ctx->m_Ptr++;
		index = 0;
		terminal = 0;

		/* Check for valid label character */
		if(false == IsStructLabelStart(ctx, *ctx->m_Ptr)) {
			error(ctx, ERR_SYNTAX, "invalid character in label");
			hasError = true;
			break;
		}

		/* Collect symbol name */
		tmp = hold;
		while(true == IsStructLabelChar(ctx, *ctx->m_Ptr)) {
			*tmp++ = *ctx->m_Ptr++;
		}
		*tmp = EOS;

		el = StructGetElement(astruct, hold);
		if(NULL == el) {
			error(ctx, ERR_INVALID_MEMBER, "'%s' is not a member of '%s'", hold, astruct->name);
			return true;
		}

		/* Process index */
		if(*ctx->m_Ptr == '[') {
			ctx->m_Ptr++;

			/* Get the index value */
			hasError = GetEvalTerm(ctx, &terminal, evalType, retSym, level);
			if(true == hasError) {
				*retResult = 0;
				return true;
			}

			/* Check for errors */
			if(*ctx->m_Ptr != ']') {
				error(ctx, ERR_SYNTAX, "subscript missing ']'");
				return true;
			} else {
				ctx->m_Ptr++;

				if(terminal >= el->count) {
					error(ctx, ERR_SUBSCRIPT_OUT_OF_RANGE, "subscript out of bounds accessing structure element");
					return true;
				}
				index = terminal;
			}
		}

		/* Add the offset */
		offset += el->offset + (el->size * terminal);

		if('.' == *ctx->m_Ptr) {
			if(NULL == el->child) {
				error(ctx, ERR_SYNTAX, "child data structure not defined");
				hasError = true;
				break;
			}
		}

		if(NULL != el->child) {
			astruct = el->child;
		}
	}

	*retResult = offset;

	return hasError;
}

/*
 *      GetEvalTerm --- evaluate a single item in an expression
 */

static bool GetEvalTerm(EnvContext *ctx,
						long *retResult,
						EVAL_TYPE evalType,
						Symbol **retSym,
						int level)
{
	long	val = 0;		/* local value being built */
	bool	negate;			/* unary minus flag */
	bool	compliment;		/* complimented value? */
	bool	hasError;

	Symbol *sym;
	Line *pnt;
	Line *bpnt;

	hasError = false;
	*retResult = 0;

	/* Check for minus sign */
	if(*ctx->m_Ptr == '-') { 
		ctx->m_Ptr++;
		negate = true;
	} else {
		negate = false;
	}

	/* Check for compliment (tilde) indicator */
	if(*ctx->m_Ptr == '~') {
		ctx->m_Ptr++;
		compliment = true;
	} else {
		compliment = false;
	}


	if(0 == strncmp(ctx->m_Ptr, "sizeof", 6)) {
		char ch = ctx->m_Ptr[6];
		if('(' == ch || '{' == ch) {
			char structName[256];
			char *dst;
			Struct *stct;

			if('(' == ch) {
				ch = ')';
			} else {
				ch = '}';
			}

			ctx->m_Ptr += 7;
			dst = structName;
			while(*ctx->m_Ptr && ch != *ctx->m_Ptr && false == IsWS(*ctx->m_Ptr)) {
				*(dst++) = *(ctx->m_Ptr++);
			}
			*dst = 0;

			if(ch != *ctx->m_Ptr) {
				error(ctx, ERR_SYNTAX, "incorrectly terminated sizeof() operator");
				hasError = true;
			}

			else {
				ctx->m_Ptr++;
				stct = StructLookup(structName);
				if(NULL == stct) {
					error(ctx, ERR_UNDEFINED, "'%s' is not defined or was not declared as a structure", structName);
					hasError = true;
				} else {
					val = stct->size;
				}
			}
		}
	}

	/*
		Check for sub expression contains in parenthesis. This needs to be
		here instead of in eval() in order to capture expressions such as
		#((lap+$444)+dog)
	*/
	else if('(' == *ctx->m_Ptr) {
		ctx->m_Ptr++;

		hasError = EvaluateSub(ctx, &val, evalType, retSym, level);
		if(false == hasError) {
			if(')' != *ctx->m_Ptr) {
				error(ctx, ERR_SYNTAX, "expression missing closing parenthesis ')'");
				hasError = true;
			}

			ctx->m_Ptr++;
		}
	}

	/* binary constant */
	else if('%' == *ctx->m_Ptr) {
		ctx->m_Ptr++;
		while(true == IsBinary(*ctx->m_Ptr)) {
			val = (val * 2) + ((*ctx->m_Ptr++) - '0');
		}
	}
	
	/* octal constant */
	else if('@' == *ctx->m_Ptr && isdigit(ctx->m_Ptr[1])) {
		ctx->m_Ptr++;
		while(true == IsOctal(*ctx->m_Ptr)) {
			val = (val * 8) + ((*ctx->m_Ptr++) - '0');
		}

	}
	
	/* hex constant - allow $... and 0x... types */
	else if('$' == *ctx->m_Ptr || ('0' == *ctx->m_Ptr && 'x' == tolower(ctx->m_Ptr[1]))) {
		ctx->m_Ptr++;
		while(true == IsHexDecimal(*ctx->m_Ptr)) {
			val *= 16;
			if(*ctx->m_Ptr > '9') {
				val += 10 + (tolower(*ctx->m_Ptr++) - 'a');
			} else {
				val += ((*ctx->m_Ptr++) - '0');
			}
		}
	}
	
	/* decimal constant */
	else if(true == IsDigit(*ctx->m_Ptr)) {
		while(*ctx->m_Ptr >= '0' && *ctx->m_Ptr <= '9') {
			val = (val * 10) + ( (*ctx->m_Ptr++)-'0');
		}
	}

	/* current location counter */
	else if('*' == *ctx->m_Ptr) {	
		ctx->m_Ptr++;
		val = GetOldPCReg();
	}
	
	
	/* character literal */
	else if('\'' == *ctx->m_Ptr) {
		ctx->m_Ptr++;
		if(*ctx->m_Ptr == EOS) {
			error(ctx, ERR_SYNTAX, "missing character constant");
			val = 0;
			hasError = true;
		} else {
			val = *ctx->m_Ptr++;
			if(*ctx->m_Ptr=='\'') {
				ctx->m_Ptr++; /* allow closing quote */
			}
		}
	}
	
	
	/* a symbol */
	else if(':' == *ctx->m_Ptr || true == IsLabelStart(ctx, *ctx->m_Ptr)) {
		char	partSymbol[MAX_BUFFERSIZE];
		char	fullSymbol[MAX_BUFFERSIZE];
		char	*dst;
		int32	offset;
		bool	hasPeriod;

		/*
			KLUDGE ALERT!!!!

			This can get a little tricky when supporting the Macro-80c dialect
			as it supports periods in symbol names.

			First we copy of the entire symbol name including the periods. This
			allows symbols containing periods to take precedence over structures.
			We then step backwards
		*/

		offset = 0;
		hasPeriod = false;
		dst = partSymbol;
		sym = NULL;

		while(':' == *ctx->m_Ptr || '.' == *ctx->m_Ptr || true == IsLabelChar(ctx, *ctx->m_Ptr)) {
			if('.' == *ctx->m_Ptr) {
				hasPeriod = true;
			}

			*(dst++) = *(ctx->m_Ptr)++;
		}

		*dst = EOS;

		if(true == hasPeriod) {

			strcpy(fullSymbol, partSymbol);

			while(NULL == sym) {

				sym = FindSymbol(ctx, partSymbol, true, false);
				if(NULL != sym) {
					break;
				}

				/* No symbol...regress down a bit */
				while(dst > partSymbol && '.' != *dst) {
					ctx->m_Ptr--;	/* Sync the operandPtr offset */
					dst--;
				}

				*dst = EOS;

				/* If we're at the end dump it */
				if(EOS == *partSymbol) {
					break;
				}
			}

			if(NULL == sym) {
				sym = FindSymbol(ctx, fullSymbol, EVAL_NOERRORS == evalType, false);
			}
		}

		else {
			sym = FindSymbol(ctx, partSymbol, EVAL_NOERRORS == evalType, false);
		}


		if(sym != NULL) {
			/*
				We need to pay particular attention to symbols in expressions
				if we are generating object files as complicated expressions
				may cause the values generated by the linker to be incorrect.

				...Currently structures are not handled!!!
			*/
			if(SYM_STRUCTURE == sym->type || SYM_UNION == sym->type) {
				hasError = GetStructEvalTerm(ctx, sym->astruct, sym->stcount, &offset, evalType, false, retSym, level);
			}

			else if(SYM_STRUCTDATA == sym->type) {
				hasError = GetStructEvalTerm(ctx, sym->astruct, sym->stcount, &offset, evalType, true, retSym, level);
				offset += sym->value;
			}

			else {
				//*retSym = sym;

				offset = sym->value;
			}


			if (ctx->m_Pass == 2) {
				pnt = sym->L_list;
				bpnt = NULL;

				while (pnt != NULL) {
					bpnt = pnt;
					pnt = pnt->next;
				}

				pnt = (Line*)AllocMem(sizeof(Line));
				if (bpnt == NULL) {
					sym->L_list = pnt;
				} else {
					bpnt->next = pnt;
				}

				pnt->L_num = ctx->m_LineNumber;
				pnt->next = NULL;
			}

			val = offset;
		}
		
		/* forward ref here */
		else if(1 == ctx->m_Pass) {
			FwdRefMark(ctx);
			val = 0;
			if(false == ctx->m_ForceByte) {
				ctx->m_ForceWord = true;
			}
		}
		
		else if(2 == ctx->m_Pass && true == FwdRefIsRecord(ctx->m_LineNumber, Cfn)) {
			if(false == ctx->m_ForceByte) {
				ctx->m_ForceWord = true;
			}
			FwdRefNext(ctx);
		}
		
		else {
			ctx->m_ForceWord = true;
			ctx->m_ForceByte = false;
		}

	}

	/* none of the above */
	else {
		if(IsWS(*ctx->m_Ptr)) {
			error(ctx, ERR_SYNTAX, "whitespace not allowed in expression");
		} else {
			error(ctx, ERR_SYNTAX, "invalid expression");
		}
		val = 0;
		hasError = true;
	}


	if(compliment == true) {
		val = ~val;
	}

	if(negate) {
		val = -val;
	}

	*retResult = val;

	return hasError;
}


/* FIXME - need to finish implementing better operator priority


static int GetOpPriority(OPERATOR op)
{
	switch(op) {
	case OP_NONE:
	case OP_OROR:
	case OP_ANDAND:
	case OP_EQ:
	case OP_NE:

	case OP_SHIFTL:
	case OP_SHIFTR:

	case OP_OR:
		return 10;

	case OP_AND:
		return 30;

	case OP_EOR:
		return 40;

	case OP_LT:
	case OP_GT:
	case OP_LTE:
	case OP_GTE:
		return 50;

	case OP_ADD:
	case OP_SUB:
		return 70;

	case OP_MUL:
	case OP_DIV:
	case OP_MOD:
		return 100;
	}
}
*/


static bool EvaluateFromLeft(EnvContext *ctx,
							 int32 left,
							 int32 *retResult,
							 EVAL_TYPE evalType,
							 Symbol **retSym,
							 int level)
{
	OPERATOR rightOp;
	OPERATOR leftOp;
	bool hasError;
	int32 right;


	if(ASM_MODE_MACRO80C == ctx->m_Compat.m_AsmMode && '!' == *ctx->m_Ptr) {
		ctx->m_Ptr++;
		left = -left;
	}

	/* Check for an operator. If not, simply return the value we were given */
	leftOp = GetOperator(ctx, true);
	if(OP_NONE == leftOp) {
		*retResult = left;
		return false;
	}

	/* Get the right term */
	hasError = GetEvalTerm(ctx, &right, evalType, retSym, level);
	if(true == hasError) {
		error(ctx, ERR_SYNTAX, "error in expression");
		return true;
	}

	if(true == ctx->m_Compat.m_UsePrecedence) {
		rightOp = GetOperator(ctx, false);


		if(OP_NONE != rightOp && rightOp > leftOp) {
			int32 rightRet;

			hasError = EvaluateFromLeft(ctx, right, &rightRet, evalType, retSym, level);
			if(true == hasError) {
				return true;
			}

			right = rightRet;
		}
	}

	switch(leftOp) {
	case OP_ADD:	left += right;			break;
	case OP_SUB:	left -= right;			break;
	case OP_MUL:	left *= right;			break;
	case OP_DIV:	left /= right;			break;
	case OP_MOD:	left %= right;			break;
	case OP_AND:	left &= right;			break;
	case OP_OR:		left |= right;			break;
	case OP_EOR:	left ^= right;			break;
	case OP_EQ:		left = (left == right);	break;
	case OP_NE:		left = (left != right);	break;
	case OP_LT:		left = (left < right);	break;
	case OP_LTE:	left = (left <= right);	break;
	case OP_GT:		left = (left > right);	break;
	case OP_GTE:	left = (left >= right);	break;
	case OP_SHIFTL:	left <<= right;			break;
	case OP_SHIFTR: left >>= right;			break;
	case OP_ANDAND:	left = left && right;	break;
	case OP_OROR:	left = left || right;	break;
	default:
		internal((ctx, "unhandled operator in expression engine"));
		break;
	}

	*retResult = left;


	return false;
}



static bool EvaluateSub(EnvContext *ctx, int32 *retResult, EVAL_TYPE evalType, Symbol **retSym, int level)
{
	long    left;
	bool	hasError;


	*retResult = 0;

	/* pickup first part of expression */
	hasError = GetEvalTerm(ctx, &left, evalType, retSym, level);
	if(true == hasError) {
		return false;
	}

	if(ASM_MODE_MACRO80C == ctx->m_Compat.m_AsmMode && '!' == *ctx->m_Ptr) {
		ctx->m_Ptr++;
		left = -left;
	}

	while(true) {
		OPERATOR top;

		top = GetOperator(ctx, false);
		if(OP_NONE == top) {
			break;
		}

		hasError = EvaluateFromLeft(ctx, left, retResult, evalType, retSym, level);
		if(true == hasError) {
			break;
		}

		left = *retResult;
	}

	*retResult = left;

	if(*ctx->m_Ptr == '[' && EVAL_INDEXED_FCB == evalType) {
		return false;
	}

	switch(*ctx->m_Ptr) {
	case EOS:
	case SPACE:
	case TAB:
	case ',':
	case ';':
	case ':':
	case ')':
	case ']':
		break;

	default:
		error(ctx, ERR_SYNTAX, "invalid character '%c' in expression", *ctx->m_Ptr);
		return true;
	}

	return false;
}


#ifdef __cplusplus
bool Evaluate(EnvContext *ctx, u_int32 *retResult, EVAL_TYPE evalType, Symbol **retSym)
{
	int32 result;
	bool status;

	status = Evaluate(ctx, &result, evalType, retSym);
	*retResult = (u_int32)result;

	return status;
}
#endif

bool Evaluate(EnvContext *ctx, int32 *retResult, EVAL_TYPE evalType, Symbol **retSym)
{
	bool hasError;


	ctx->m_ForceByte = false;
	ctx->m_ForceWord = false;
	
	/* Skip any '#' characters */
	while(*ctx->m_Ptr == '#') {
		ctx->m_Ptr++;
	}

	/* Check for forward of backward reference */
	if(*ctx->m_Ptr == '<') {
		ctx->m_ForceByte = true;
		ctx->m_Ptr++;
	} else if(*ctx->m_Ptr == '>') {
		ctx->m_ForceWord = true;
		ctx->m_Ptr++;
	}


	hasError = EvaluateSub(ctx, retResult, evalType, retSym, 0);

	return(true == hasError ? false : true);
}

