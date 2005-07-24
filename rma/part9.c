
/* Part9.c - for "RMA"

 * Most of these seem to be MACRO - related
 */

#include "rma.h"

         /*   "place" for fseek     */
#define BEGIN 0
#define HERE 1
#define END 2

struct macro_ptr {
   FILE *m_ptr;
   long PrevMpos;
   int fi1,
       fi2;
} MacPtrs[8];

/*extern direct unsigned _IfIsTru, ListLin;
extern direct char x_flg, Pass2, TmpNam;
extern direct char *SrcChar, *Label, *operatr, *oprand;
extern direct struct symblstr *SmblDsc;
extern direct int InMacro, MacFlag, d00da;
extern direct MACDSCR *now_mac;
extern direct int d00d8;
extern char d0821;*/

   /* NOTE: study this: I THINK that d00e0 - d0038 MAY be
            struct macro_ptr !!!!!    */
direct int  d00e0;          /* FILE *m_ptr; */
direct char *d00e2;         /* long PrevMpos;    ??????????????? */
direct int  mac_dpth;       /* D00e4 */
direct char *d00e6;         /* int fi1,     */
/*  NOTE!!!  d00e8 MUST be a structure... */
direct int  *d00e8;         /* fi2;         */

#ifdef __STDC__

int 
newMac (void)
#else

newMac()
#endif
                           /* l3183 */
{
 register MACDSCR *macdsc;
 char *var;

   if ( PASS1 ) {
      if ( macfp1 == 0 ) {
         if ( (macfp2 = macfp1 = fopen( TmpNam, "w+")) == 0 ) {
            errexit( "can't open macro work file" );
         }
	 /* Let's forget this for now.. can't seem to understand how it's
	  * done in Linux...
	  */
         /*macfp1->_bufsiz = 512;*/
      }
      macdsc = (MACDSCR *)getmem( sizeof(MACDSCR) );
      macdsc->LastMac = now_mac;
      now_mac = macdsc;
      macdsc->MSblPtr= SmblDsc;
      macdsc->MWrkFil= macfp1;
      fseek( macfp1, 0l, END );
      var = SmblDsc->symblnam;
      while ( *var ) {
         putc( *(var++), macfp1 );
      }
      putc ( '\0', macfp1 );
      if ( ferror(macfp1) ) {
         mfilerr();
      }
      macdsc->MacStart = ftell(macfp1);
   }
   return 1;
}

#ifdef __STDC__

int 
mcWrtLin (void)
#else

mcWrtLin()
#endif
{
   if ( PASS1 ) {
      putfield( Label, ' ' );
      putfield( operatr, ' ' );
      putfield( SrcChar, '\n' );
   }
   return 1;
}

#ifdef __STDC__

int 
putfield (          /* l324c() */
    char *lin,
    int cchr
)
#else

putfield( lin, cchr )          /* l324c() */
 char *lin;
 char cchr;
#endif
{
 register char *reg;

   if ( (reg=lin) ) {
      while ( *reg != '\0' ) {
         putc( *(reg++), macfp1 );
      }
   }
   putc( cchr, macfp1 );
   if ( ferror(macfp1) ) {
      mfilerr();
   }
}

#ifdef __STDC__

int 
add_nul (void)
#else

add_nul()
#endif
{
   if ( PASS1 ) {
      putc ( '\0', macfp1 );
   }
   return 1;
}

/* NOTE: This MSblPtr[10] stuff is wrong!!!
         temp fix  till symblstrct gets fixed   */

#ifdef __STDC__

MACDSCR *
McNamCmp (char *parm)
#else

McNamCmp( parm )
 char *parm;
#endif
{
 register MACDSCR *reg = now_mac;

   while ( reg ) {
      if ( parm[0] == (reg->MSblPtr)->symblnam[0] ) {
         if ( strcmp( parm, ((reg->MSblPtr)->symblnam) ) == 0 ) {
            return reg;
         }
      }
      reg = reg->LastMac;
   }
   return 0;
}

#ifdef __STDC__

int 
addMac (
/* struct macro_ptr *parm;*/
    MACDSCR *parm
)
#else

addMac( parm )
/* struct macro_ptr *parm;*/
   MACDSCR *parm;
#endif
{
 register struct macro_ptr *reg = &MacPtrs[mac_dpth++];

   if ( mac_dpth > 8 ) {
      errexit( "macro nesting too deep" );
   }
   reg->m_ptr = macfp2;
   reg->PrevMpos = ftell(macfp2);
   reg->fi1 = d00e0;
   reg->fi2 = d00e2;
   macfp2 = parm->MWrkFil;
   fseek( macfp2, parm->MacStart, BEGIN );
   if ( InMacro == 0 ) {
      ++d00da;
   }
   d00e0 = d00da;
   d00e2 = l34a8();
   MacFlag = 1;
}

#ifdef __STDC__

int 
decMac (void)
#else

decMac()
#endif
{
 register struct macro_ptr *reg = &MacPtrs[--mac_dpth];

   if ( mac_dpth  < 0 ) errexit( "asm err: macro nest" );
   macfp2 = reg->m_ptr;
   fseek( macfp2, reg->PrevMpos, BEGIN );
   d00e0 = reg->fi1;
   l35df(d00e2);
   d00e2 = reg->fi2;
}

#ifdef __STDC__

int 
readmacs (void)
#else

readmacs()
#endif
{
 char mac_chr;

   while ( MacFlag ) {
      d00e6 = SrcChar = d0821;

      while ( (mac_chr = getc(macfp2)) > '\0' ) {
         if ( mac_chr == '\n' ) {
            *d00e6 = '\0';
            if ( x_flg ) {
               ++ListLin;
            }
            return 1;
         }
         if ( (_IfIsTru == 0) && ( mac_chr == '\\' ) ) {
            l3402( getc(macfp2) );
         }
         else {
            *(d00e6++) = mac_chr;
         }
      }
      if ( mac_dpth == 0 ) {
         MacFlag = 0;
         break;
      }
      decMac();
   }
   return 0;
}

#ifdef __STDC__

int 
l3402 (int parm)
#else

l3402(parm)
 char parm;
#endif
{
   if ( isdigit(parm) ) {
      gArgParm( parm, 1 );
   }
   else {
      switch (parm) {
         case '@':
            *(d00e6++) = parm;
            sprintf( d00e6, "%05u", d00e0 );
            d00e6 += 5;
            break;
         case 'L':
         case 'l':
         case '#':
            sprintf( d00e6, "%02d",
                    ( parm == '#' ? *d00e2 : gArgParm(getc(macfp2), 0) ) );
            d00e6 += 2;
            break;
         default:
            *(d00e6++) = parm;
      }
   }
}

#ifdef __STDC__

int 
l34a8 (void)
#else

l34a8()
#endif
{
 register char *reg;
 char *var1;
 int MArgCnt, arglen;
 char mquote, argchar;

   MArgCnt = 0;
   arglen = 59;
   if ( d00e8 ) {
      reg = d00e8;
      d00e8 = *d00e8;
   }
   else {
      reg = getmem( 61 );
   }
   SkipSpac();
   oprand = SrcChar;
   d00d8 = 1;
   var1 = reg++;

   if ( *SrcChar != '\0' ) {
      while ( arglen && ((argchar = *SrcChar) != '\0') && (argchar != ' ') ) {
         switch ( *SrcChar ) {
            case ',':
               *(reg++) = '\0';
               ++MArgCnt;
               ++SrcChar;
               break;
            case ' ':
               goto endloop;
            case '\'':
            case '\"':
               mquote = *(SrcChar++);
               while ( ((argchar = *(SrcChar++)) != '\0') &&
                               (argchar != mquote) ) {
                  if ( argchar == '\\' ) {
                     argchar = *(SrcChar++);
                  }
                  *(reg++) = argchar;
               }
               if ( argchar == '\0' ) {
                  e_report( "unmatched quotes" );
               }
               break;
            case '\\':
               ++SrcChar;
               *(reg++) = *(SrcChar++);
               break;
            default:
               *(reg++) = *(SrcChar++);
         }
         --arglen;
      }
      ++MArgCnt;
   }

endloop:
   *reg = '\0';
   if ( arglen == '\0' ) {
      e_report( "macro arg too long" );
   }
   if ( MArgCnt > 9 ) {
      e_report( "too many args" );
   }
   *var1 = MArgCnt;
   return var1;
}

#ifdef __STDC__

int 
l35df (int *parm)
#else

l35df(parm)
 int *parm;
#endif
{
   *parm = d00e8;
   d00e8 = parm;
}

#ifdef __STDC__

int 
gArgParm (int m_count, int parm2)
#else

gArgParm(m_count, parm2)
 int m_count;
#endif
{
 int var = 0;
 register char *reg;

   if ( (m_count -= '0') > 0 ) {
      if ( *(reg=d00e2) >= m_count ) {
         ++reg;
         while ( --m_count > 0 ) {
            while ( *(reg++) ){}
         }
         while ( *reg != '\0' ) {
            if ( parm2 != '\0' ) {
               *(d00e6++) = *reg;
            }
            ++reg;
            ++var;
         }
      }
      else {
         e_report( "no param for arg" );
      }
   }
   return var;
}

#ifdef __STDC__

int 
closmac (void)
#else

closmac()
#endif
{
	/* original version didn't have this if test in it
	 * However, Linux sqawks with segfault if macfp1 == 0
	 * This is the correct way */
	if(macfp1) {
		fclose(macfp1);
		unlink(&TmpNam);
	}
}

#ifdef __STDC__

int 
mfilerr (void)
#else

mfilerr()
#endif
{
   errexit( "macro file error" );
}

