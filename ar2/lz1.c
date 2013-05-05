
/*
 *------------------------------------------------------------------
 *
 * $Source$
 * $RCSfile$
 * $Revision$
 * $Date$
 * $State$
 * $Author$
 * $Locker$
 *
 *------------------------------------------------------------------
 *
 * Carl Kreider
 * 22305 CR 28
 * Goshen, IN  46526
 * (219) 875-7019
 * (71076.76@compuserve.com, ckreider@skyenet.net, carlk@syscon-intl.com)
 *
 *------------------------------------------------------------------
 * $Log$
 * Revision 1.5  2008/11/03 15:02:20  robertgault
 * added a WIN32 include of gmon.h
 *
 * Revision 1.4  2008/10/30 15:52:24  boisy
 * Clenaed up warnings in ar2
 *
 * Revision 1.3  2006/09/09 01:59:03  boisy
 * Changes to accomodate compiling under Turbo C++
 *
 * Revision 1.2  1996/07/20 22:24:34  cc
 * Merged in pwz's unixification (Sunos).
 *
 * Revision 1.1  96/07/20  17:10:37  cc
 * Initial revision
 * 
 *------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#ifdef SYSV
# include <sys/types.h>
#else
# include <types.h>
#endif
#include "arerrs.h"
#include "lz1.h"
#ifdef WIN32
# include <gmon.h>
#endif

void insert_bit(short code);
void addentry(WORD c, WORD ent);
void writebuf(int cnt, FILE *fp);
void lz1_init(int direction);
char *emalloc(size_t);

/*page*/
/*
 *      Writes compressed file to outfile.
 */

int LZ_1(FILE *infile, FILE *outfile, long *bytes)
	{
	VOID				output();
	WORD				c, ent, tag = TAG;
	WORD				n_ent;
	register COMPTBL	*ctp;

	lz1_init(COMP);
	lz_bytes = sizeof(tag);
	writeshort(outfile, tag);			/* mark as LZ					*/

	ent = getc(infile);
	while (!feof(infile) && (c = getc(infile)) != EOF)
		{
		/*
		 * Find the entry corresponding to the current entry suffixed
		 * with c.  Since the entries are sorted, suffix > c is as
		 * good as a null to indicate the need to create a new entry.
		 */
		n_ent = CompTbl[ent].chain;
		for (; ; )
			{
			ctp = &CompTbl[n_ent];
			if ((n_ent == 0) || (ctp->suffix > c))
				{
				output(ent, outfile);
				addentry(c, ent);		/* try to grow the dictionary	*/

				ent = c;
				break;
				}
			else
				if (ctp->suffix != c)
					n_ent = ctp->next;
				else
					{
					ent = n_ent;
					break;
					}
			}
		}

	output(ent, outfile);				/* put out final code			*/
	output(-1, outfile);				/* and -1 to flush and finish	*/
	*bytes = lz_bytes;
	return (0);
	}
/*page*/
/*
 * We add an entry to the table if we can.  If not, we just return.
 */

void addentry(WORD c, WORD ent)
	{
	register COMPTBL	*ctp = CompTbl;
	COMPTBL				*fep, *cep;
	WORD				p_ent;

	/* if the table is full, there's nothing we can do.	*/
	if (free_ent < maxmaxcode)
		{
		fep = &ctp[free_ent];
		fep->chain = 0;
		fep->suffix = c;
		cep = &ctp[ent];

		if (((p_ent = cep->chain) == 0) || (c < ctp[p_ent].suffix))
			{
			fep->next = p_ent;
			cep->chain = free_ent;
			}
		else
			{
			while (((ent = ctp[p_ent].next) !=0) && (c >= ctp[ent].suffix))
				p_ent = ent;

			fep->next = ent;
			ctp[p_ent].next = free_ent;
			}

		free_ent++;
		}
	}
/*page*/
/*
 * Output the given code.
 */

VOID	output(code, ofp)
WORD	code;
FILE	*ofp;
	{
	if (code < 0)
		{
		/* at EOF--flush buffers and pack up	*/
		if (offset > 0)
			writebuf((offset + 7) >> 3, ofp);

		fflush(ofp);
		}
	else
		{
		insert_bit(code);

		if ((offset += n_bits) == BytesToBits(n_bits))
			writebuf(n_bits, ofp);

		/*
		 * If the next entry is going to be too big for the code size,
		 * then increase it, if possible.
		 */
		if (free_ent > maxcode)
			{
			/*
			 * Write the whole buffer, because the input side won't
			 * discover the size increase until after it has read it.
			 */
			if (offset > 0)
				writebuf(n_bits, ofp);

			n_bits++;
			maxcode = (n_bits == maxbits) ? maxmaxcode : (1 << n_bits) - 1;
			}
		}
	}
/*page*/
/*
 * function to write the buffer
 */

void writebuf(int cnt, FILE *fp)
	{
	register UWORD	*bp = buf;
	int				lim;

	for (lim = (cnt >> 1); lim; --lim)
		writeshort(fp, *bp++);

	if (cnt & 1)
		putc((*bp >> 8) & 0xff, fp);

	lz_bytes += cnt;
	offset = 0;
	}


WORD mask1[] = {
	0x0000, 0x8000, 0xc000, 0xe000,
	0xf000, 0xf800, 0xfc00, 0xfe00,
	0xff00, 0xff80, 0xffc0, 0xffe0,
	0xfff0, 0xfff8, 0xfffc, 0xfffe};

WORD mask2[] = {
	0x0000, 0x0001, 0x0003, 0x0007,
	0x000f, 0x001f, 0x003f, 0x007f,
	0x00ff, 0x01ff, 0x03ff, 0x07ff,
	0x0fff, 0x1fff, 0x3fff, 0x7fff};

/*
 * insert a code of "n_bits" bits at "offset" bits into buf
 */

void insert_bit(short code)
	{
	register UWORD	*bufp;
	short			t1, w_offset, shift, size2;

	bufp = &buf[(offset >> 4)];
	w_offset = offset & 0x0f;

	if ((t1 = w_offset + n_bits) <= WSIZE)
		{
		shift = WSIZE - t1;
		size2 = n_bits;
		}
	else
		{
		size2 = t1 - WSIZE;
		shift = (WSIZE * 2) - t1;
		*bufp = (*bufp & mask1[w_offset]) | ((unsigned) code >> size2);
		++bufp;
		}

	*bufp = (*bufp & ~(mask2[size2] << shift)) | (code << shift);
	}
/*page*/
/*
 * Decompress the input file.
 */

WORD	de_LZ_1(infile, outfile, bytes)
FILE	*infile, *outfile;
long	bytes;
	{
	register DCOMPTBL	*dtp;
	u_char				stack[MAXSTACK];
	WORD				tag, finchar, code, oldcode, incode;
	register u_char		*stackp = stack;
	WORD				getcode();

	if (readshort(infile, &tag) == EOF || tag != TAG)
		return (NOT_AR);

	lz1_init(DECOMP);
	dtp = CrakTbl;
	lz_bytes = bytes - sizeof(tag);

	if ((finchar = oldcode = getcode(infile)) == EOF)
		return (EOF);

	if (putc((u_char) finchar, outfile) == EOF)		/* first code is 8 bits	*/
		return (WERR);

	while ((incode = code = getcode(infile)) >= 0)
		{
		if (code >= free_ent)			/* Special case for KwKwK string.	*/
			{
			*stackp++ = finchar;
			code = oldcode;
			}

		while (code >= 256)			/* Generate characters in reverse order	*/
			{
			*stackp++ = dtp[code].lastch;
			code = dtp[code].prefix;
			}
		*stackp++ = finchar = dtp[code].lastch;

		/* And put them out in forward order	*/
		do	{
			if (putc(*--stackp, outfile) == EOF)
				return (WERR);
			} while (stackp > stack);

		if ((code = free_ent) < maxmaxcode)			/* Generate new entry	*/
			{
			dtp[code].prefix = oldcode;
			dtp[code].lastch = finchar;
			free_ent++;
			}

		oldcode = incode;						/* Remember previous code.	*/
		}

	return (code);
	}
/*page*/
/*
 * Read one code from the input file.  If EOF, return -1.
 */

WORD	getcode(infile)
FILE	*infile;
	{
	WORD		code, reslt;
	static WORD	size = 0;

	if ((offset >= size) || (free_ent > maxcode))
		{
		if (free_ent > maxcode)
			{
			n_bits++;					/* new entry too big, increase size	*/
			maxcode = (n_bits == maxbits) ? maxmaxcode : (1 << n_bits) - 1;
			}

		if (lz_bytes <= 0)
			return (EOF);				/* "eof"						*/

		size = (lz_bytes > (long) n_bits) ? n_bits : (int) lz_bytes;
		reslt = readbuf(size, infile);	/* read new buffer too			*/
		if (reslt == EOF)
			return (EOF);

		lz_bytes -= size;
		offset = 0;
		/*
		 * Convert size to bits, and round down so that stray bits
		 * at the end aren't treated as data
		 */
		size = BytesToBits(size) - (n_bits - 1);
		}

	code = fetch();
	offset += n_bits;
	return (code);
	}


int fetch(void)
	{
	register UWORD	*bp;
	register WORD	t1, t2, w_offset;

	bp = &buf[offset >> LOG2WSIZE];
	w_offset = offset & LowOrder(LOG2WSIZE);

	if ((t1 = w_offset + n_bits) <= WSIZE)
		t2 = (*bp >> (WSIZE - t1));
	else
		{
		t2 = (*bp++ << (t1 - WSIZE));
		t2 |= (*bp >> (32 - t1));
		}

	return (t2 & LowOrder(n_bits));
	}
/*page*/
/*
 * function to fill the buffer
 */

int readbuf(int cnt, FILE *fp)
	{
#ifdef OSK
	register char	*bp = (char *) buf;
	register int	c;

	while (cnt--)
		if (EOF != (c = getc(fp)))
			*bp++ = c;
		else
			return (c);

#else
	register UWORD	*bp = buf;
	int				lim;
	UWORD			word;

	for (lim = (cnt >> 1); lim; --lim)
		if (readshort(fp, &word) == EOF)
			return (EOF);
		else
			*bp++ = word;

	/* note that we use 'lim' here to get a signed variable */
	if (cnt & 1)
		if ((lim = getc(fp)) == EOF)
			return (EOF);
		else
			*bp = (lim << 8);
#endif

	return (0);
	}
/*page*/
/*
 * routines to initialize the Lempel-Zev version one routines
 */

void lz1_init(int direction)
	{
	n_bits = INIT_BITS;
	maxcode = (1 << n_bits) - 1;
	maxmaxcode = (1 << maxbits) - 1;
	if (direction == COMP)
		{
		register COMPTBL	*ctp;

		if ((ctp = CompTbl) == NULL)
#ifdef DEBUG
			{
			int		n = ((1 << maxbits) * sizeof(COMPTBL));
			ctp = CompTbl = (COMPTBL *)emalloc(n);
			memset(ctp, 0, n);
			}
#else
			ctp = CompTbl = (COMPTBL *)emalloc((1 << maxbits) * sizeof(COMPTBL));
#endif

		for (free_ent = 0; free_ent < 256; free_ent++)
			{
			ctp->next = ctp->chain = 0;
			ctp->suffix = free_ent;
			++ctp;
			}
		}
	else
		{
		register DCOMPTBL	*dtp;

		if ((dtp = CrakTbl) == NULL)
#ifdef DEBUG
			{
			int		n = ((1 << maxbits) * sizeof(DCOMPTBL));
			dtp = CrakTbl = (DCOMPTBL *)emalloc(n);
			memset(dtp, 0, n);
			}
#else
			dtp = CrakTbl = (DCOMPTBL *)emalloc((1 << maxbits) * sizeof(DCOMPTBL));
#endif

		for (free_ent = 0; free_ent < 256; free_ent++)
			{
			dtp->prefix = 0;
			dtp->lastch = free_ent;
			++dtp;
			}
		}
	}


/*
 * configure lz1 for number of bigs
 */

int lz1_config(int bits)
	{
	if (bits)
		{
		if ((bits < INIT_BITS) || (bits > BITS))
			{
			fprintf(stderr, " bits must be in %d..%d\n", INIT_BITS, BITS);
			exit (1);
			}

		maxbits = bits;
		}

	return (maxbits);
	}
#ifdef DEBUG
/*page*/
dump_itbl()
	{
	short	nxt, chn, suf;
	int		i, lim = maxmaxcode + 1;

	for (i = 0; i < lim; i += 1)
		{
		nxt = CompTbl[i].next;
		chn = CompTbl[i].chain;
		suf = CompTbl[i].suffix;
		if (nxt || chn || ((i > 256) && suf))
			{
			if ((32 <= nxt) && (nxt < 127))
				fprintf(stderr, "%03x  %03x <%c>", i, nxt, nxt);
			else
				fprintf(stderr, "%03x  %03x    ", i, nxt);
			
			if ((32 <= chn) && (chn < 127))
				fprintf(stderr, "  %03x <%c>", chn, chn);
			else
				fprintf(stderr, "  %03x    ", chn);
			
			if ((32 <= suf) && (suf < 127))
				fprintf(stderr, "  %03x <%c>\n", suf, suf);
			else
				fprintf(stderr, "  %03x    \n", suf);
			}
		}
	}


dump_otbl()
	{
	short	ch, pref;
	int		i, lim = maxmaxcode + 1;

	for (i = 0; i < lim; i += 1)
		{
		pref = CrakTbl[i].prefix;
		ch = CrakTbl[i].lastch;
		if (pref || ((i > 256) && ch))
			{
			if ((32 <= pref) && (pref < 127))
				fprintf(stderr, "%03x  %03x <%c>", i, pref, pref);
			else
				fprintf(stderr, "%03x  %03x    ", i, pref);
			
			if ((32 <= ch) && (ch < 127))
				fprintf(stderr, "  %03x <%c>\n", ch, ch);
			else
				fprintf(stderr, "  %03x    \n", ch);
			}
		}
	}
#endif
