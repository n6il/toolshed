/* Part6.c - mostly deals with Labels */

#include "rma.h"

#define _ischar(c) (_chcodes[c]&(_DIGIT|_LOWER|_UPPER|_CONTROL))

/*
 * extern direct long nmbr_int; extern direct unsigned HadArrow, IsExtrn;
 * extern direct char **S_Addrs, *CodeSize, Pass2, d005e, d005f, *SrcChar;
 * extern direct short GlobCnt; extern direct char LblTyp; extern direct
 * struct symblstr **d0078, *SmblDsc;
 * 
 * extern char    *AltLbl; extern struct symblstr d0587;
 * 
 * extern struct symblstr *d078f, **d07b1; extern struct ref_ent d0791[],
 * *OptBPtr;
 */

/* extern char *_chcodes; */


/* *********************************** */

/*
 * Moves the label at SrcChar to destpos
 *
 * Returns 1 of copy was successful.
 */
#ifdef __STDC__

int
                MoveLabel(char *destpos)
#else

MoveLabel(destpos)
	char           *destpos;

#endif
{
	int             count;

	if (isalpha(*SrcChar))
	{
		count = SYMLEN - 1;
		while ((count-- > 0) && (_ischar(*SrcChar)))
		{
			*(destpos++) = *(SrcChar++);
		}
		*destpos = '\0';
		while (_ischar(*SrcChar))
		{
			++SrcChar;
		}
		return 1;
	}
	return 0;
}

#ifdef __STDC__

int
                l1e4d(void)
#else

l1e4d()
#endif
{
	register struct symblstr *newtree;
	int             strrel;

	SmblDsc = 0;
	if (MoveLabel(AltLbl) == 0)
	{
		return 0;
	}
	if (*SrcChar == ':')
	{
		++SrcChar;
		LblTyp |= 4;
	}
	strrel = FndTreEmty(AltLbl, &SmblDsc);
	newtree = SmblDsc;

	if (PASS1)
	{
		if (strrel)
		{
			SmblDsc = newtree = TreSetUp(AltLbl, &SmblDsc, strrel);
		}
		if ((newtree->w1 & RELATIVE) == 0)
		{
			newtree->smbltyp = (newtree->smbltyp & 0xf0) | d005f;
			newtree->w1 |= LblTyp;
			if (S_Addrs)
			{
				newtree->s_ofst = *S_Addrs;
			}
		}
		else
		{
			if ((newtree->smbltyp & 0x0f) != LT_SET)
			{
				newtree->w1 |= NEGMASK;
			}
		}
		if ((newtree->w1 & 0xc6) == (RELATIVE | CODENT | DIRENT))
		{
			++GlobCnt;
		}
		return 1;
	}
	if (strrel || ((newtree->w1 & RELATIVE) == 0))
	{
		errexit("symbol lost!?");
	}
	if (((newtree->w1 &= 0xef) & NEGMASK))
	{
		e_report("redefined name");
		return 1;
	}
	else
	{
		if (S_Addrs && ((newtree->smbltyp & 0x0f) == d005e) &&
		    (newtree->s_ofst != *S_Addrs))
		{
			phaserr();
			newtree->s_ofst = *S_Addrs;
		}
	}
	return 1;
}

#ifdef __STDC__

int
                l1f63(void)
#else

l1f63()
#endif
{
	register struct symblstr *entry;
	struct ref_str *s_ref;
	int             cmp_case,
	                s_type;

	cmp_case = FndTreEmty(d0587, &SmblDsc);
	entry = SmblDsc;
	if (cmp_case)
	{
		if (PASS1)
		{
			entry = SmblDsc = TreSetUp(d0587, &SmblDsc, cmp_case);
		}
		else
		{
			errexit("new symbol in pass two");
		}
	}
	s_type = entry->smbltyp & 0x0f;
	if ((entry->w1 ^ RELATIVE) & (RELATIVE | CODLOC))
	{
		IsExtrn = 1;
		if (PASS1)
		{
			entry->w1 |= DIRLOC;
		}
		if ((entry->w1 & RELATIVE) && ((s_type == LT_EQU) ||
					       (s_type == LT_SET)))
		{
			for (s_ref = entry->wrd; s_ref;)
			{
				OptBPtr->ETyp = s_ref->RfTyp;
				SetArow((void*)(OptBPtr->RAddr = (void*)s_ref->r_offset));
				++OptBPtr;
				s_ref = s_ref->NxtRef;
			}
		}
		else
		{
			OptBPtr->ETyp = 0;
			(OptBPtr++)->RAddr = (void*)entry;
			SetArow(entry);
		}
	}
	else
	{
		if (Pass2)
		{
			if (entry->w1 & DIRLOC)
			{
				SetArow(entry);
			}
		}
	}
	if (s_type == 7)
	{
		nmbr_int = 0;
	}
	else
	{
		nmbr_int = entry->s_ofst;
	}
	return 1;
}

#ifdef __STDC__
void
                SetArow(	/* L204a() */
			                struct symblstr * parm	/* parm is some type of
								 * struct ptr */
)
#else

void
                SetArow(parm)	/* L204a() */
	struct symblstr *parm;	/* parm is some type of struct ptr */

#endif
{
	register struct symblstr *sbl = parm;

	if (!HadArrow)
	{
		if ((sbl->smbltyp & 6) == 2)
		{
			HadArrow = 1;
		}
		else
		{
			HadArrow = -1;
		}
	}
}

#ifdef __STDC__

int
                l2069(int parm)
#else

l2069(parm)
	char            parm;

#endif
{
	if (((SmblDsc->w1) & NEGMASK) == 0)
	{
		if ((parm != LT_SET) && ((SmblDsc->smbltyp & 0x0f) == LT_SET))
		{
			(SmblDsc->w1) |= NEGMASK;
		}
		else
		{
			SmblDsc->smbltyp = ((SmblDsc->smbltyp & 0xf0) | parm);
		}
	}
}

#ifdef __STDC__
void
                l20a4(_ushort address)
#else
void
                l20a4(address)
	_ushort         address;

#endif
{
	register struct symblstr *reg = SmblDsc;
	struct ref_ent *var1;
	struct ref_str *var2;
	int             var3;

	var3 = reg->smbltyp & 0x0f;

	if (OptBPtr == d0791)
	{
		if (((reg->smbltyp & CODENT) != 0) && ((reg->w1 & CODLOC) != 0))
		{
			reg->w1 &= 0xdf;
		}
		else
		{
			if ((Pass2 > 0) && (address != reg->s_ofst) &&
			    !(reg->w1 & NEGMASK) && (var3 != LT_SET))
			{
				phaserr();
			}
		}
	}
	else
	{
		if (PASS1)
		{
			if (var3 == LT_EQU)
			{
				reg->w1 |= CODLOC;
				while (OptBPtr > d0791)
				{
					--OptBPtr;
					RefCreat(reg, OptBPtr->ETyp, (int)OptBPtr->RAddr);
				}
			}
		}
		else
		{		/* Pass 2 */
			if (var3 == LT_EQU)
			{
				var2 = reg->wrd;
				var1 = OptBPtr;
				while (var2 && (d0791 < var1))
				{
					--var1;
					if ((var2->r_offset != (void*)var1->RAddr))
					{
						break;
					}
					var2 = var2->NxtRef;
				}
				if ((var2 != 0) || (d0791 != var1) ||
				    (reg->s_ofst != (int) address))
				{
					phaserr();
				}
				else
				{
					if (reg->w1 & 4)
					{
						var2 = reg->wrd;
						while (var2)
						{
							if (((var2->r_offset->w1) & 0x42) == 0)
							{
								ilExtRef();
							}
							var2 = var2->NxtRef;
						}
					}
				}
			}
			else
			{
				ilExtRef();
			}
		}
	}
	reg->s_ofst = address;
}

#ifdef __STDC__

static
                phaserr(void)
#else

static          phaserr()
#endif
{
	e_report("phasing error");
}

#ifdef __STDC__

int
                RefCreat(	/* L21c9()  */
			                 struct symblstr * myref,
			                 int rtyp,
			                 int adrs
)
#else

RefCreat(myref, rtyp, adrs)	/* L21c9()  */
	struct symblstr *myref;
	char            rtyp;
	int            *adrs;

#endif
{
	register struct ref_str *new;

	/* new = getmem(5); */
	new = (struct ref_str *) getmem(sizeof(struct ref_str));
	new->RfTyp = rtyp;
	new->r_offset = (void*)adrs;
	new->NxtRef = myref->wrd;
	myref->wrd = new;
}


/*
 * straight asm, how do we do otherwise ??? EASY!!!!!!!!!!!!!
 */
#ifdef __STDC__

char
                SkipSpac(void)
#else

SkipSpac()
#endif
{
	register char  *pt = SrcChar;

#if 0
	while (*(pt++) == ' ');	/* Loop till non-space */

	return *(SrcChar = --pt);	/* Reset SrcChar to this char */
#else
	while (*pt == ' ' || *pt == '\t')
	{
		pt++;
	}

	return *(SrcChar = pt);	/* Reset SrcChar to this char */
#endif
}

/*
#ifdef COCO
#asm
SkipSpac: ldx SrcChar
l21e7 ldb ,x+
 beq l21ef
 cmpb #$20
 beq l21e7
l21ef sex
 leax -1,x
 stx SrcChar
 rts
#endasm
#else
#asm
SkipSpac: move.l a0,-(sp)
 movea.l SrcChar(a6),a0
l21e7 move.b (a0)+,d0
 beq l21ef
 cmpi.b #$20,d0 space?
 beq l21e7
l21ef ext.w d0
 ext.l d0
 lea.l -1(a0),a0
 move.l a0,SrcChar(a6)
 move.l (sp)+,a0
 rts
#endasm
#endif
*/
#ifdef __STDC__

struct symblstr *
                WlkTreLft(struct symblstr * thistree)
#else

struct symblstr *WlkTreLft(thistree)
	struct symblstr *thistree;

#endif
{
	register struct symblstr *ntree = thistree->right;

	if (thistree->w1 & 8)
	{
		while (ntree->left)
		{
			ntree = ntree->left;
		}
	}
	return ntree;
}

#ifdef __STDC__

struct symblstr *
                TreSetUp(char *newnam, struct symblstr ** oldtree, int cmp_cas)
#else

struct symblstr *TreSetUp(newnam, oldtree, cmp_cas)
	char           *newnam;
	struct symblstr **oldtree;
	int             cmp_cas;

#endif
{
	register struct symblstr *nwtree;
	struct symblstr *var;

	if (d078f)
	{
		nwtree = d078f;
		d078f = d078f->left;

		/*
		 * nwtree->left = nwtree->wrd = nwtree->smbltyp = nwtree->w1
		 * = nwtree->s_ofst = 0;
		 */
		memset(nwtree, 0, sizeof(struct symblstr));	/* easier */
	}
	else
	{
		nwtree = (struct symblstr *) getmem(sizeof(struct symblstr));

		/*
		 * nwtree->left = nwtree->right = nwtree->wrd =
		 * nwtree->smbltyp = nwtree->w1 = nwtree->s_ofst = 0;
		 */
		memset(nwtree, 0, sizeof(struct symblstr));
	}
	if (var = *oldtree)
	{			/* if an oldtree exists */
		if (cmp_cas < 0)
		{
			nwtree->right = var;
			var->left = nwtree;
		}
		else
		{
			nwtree->right = var->right;
			var->right = nwtree;
			var->w1 |= 8;
		}
	}
	else
	{
		*d07b1 = nwtree;
		nwtree->right = 0;
	}
	strcpy(nwtree->symblnam, newnam);
	return nwtree;
}

/*
 * FndTreEmty() -Trace Tree down to find an empty space at the right place
 * and return strcmp() result, or, return 0 if this name has already been
 * entered
 */

#ifdef __STDC__

int
                FndTreEmty(	/* var = L227f( AltLbl, &SmblDsc )        */
			                   char *new_nam,
			                   struct symblstr ** tptr
)
#else

FndTreEmty(new_nam, tptr)	/* var = L227f( AltLbl, &SmblDsc )        */
	char           *new_nam;
	struct symblstr **tptr;

#endif
{
	int             cmp_case = 1;

	{
		register char  *place = new_nam;
		int             sum = 0;

		do
		{
			sum += *place;
		} while (*(++place) != '\0');

		/* d07b1 = ((sum & 0x3f)<<1) + d0078; */

		/*
		 * The following doesn't seem exactly right, but it works
		 * (for OSK) what we're taking is the contents of d0078 and
		 * adding to it sum, etc - d0078 seems to be a pointer to an
		 * array of struct smblstructs
		 */

		d07b1 = &d0078[sum & 0x3f];
	}

	{
		register struct symblstr *treep;

		if (treep = *tptr = *d07b1)
		{
			do
			{
				if ((cmp_case = strcmp(new_nam, ((*tptr = treep)->symblnam))) < 0)
				{
					treep = treep->left;
				}
				else
				{
					if ((cmp_case <= 0) || ((treep->w1 & 8) == 0))
					{
						break;
					}
					treep = treep->right;
				}
			} while (treep);

		}
		return cmp_case;
	}
}

getmem(memreq)
	int             memreq;
{
	int             memgot;

#ifndef COCO
	if ((memgot = (int)malloc(memreq)))
	{
#else
	if ((memgot = sbrk(memreq)) != -1)
	{
#endif
		return memgot;
	}
	else
	{
		errexit("symbol table overflow");
	}
}
