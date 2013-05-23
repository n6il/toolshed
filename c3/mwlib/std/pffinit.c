/*
     Modification history:
          20-May-83 Fixed rounding problem causeing numbers to
                    print with colons.
*/

#include <stdio.h>
#include <ctype.h>

extern double scale();
extern double ftoatbl[],
              *ftblend;
static char buf[30];

pffinit()
{
}

pffloat(c,prec,args)
double **args;
{
        int format;

        switch(c) {
                case 'f':
                        format = 1;
                        break;
                case 'e':
                case 'E':
                        format = -1;
                        break;
                case 'g':
                case 'G':
                        format = 0;
        }
        return _ftoa((*args)++,prec,format,isupper(c));
}

static _ftoa(np0,prec,format,c)    /* double to ascii conversion */
double *np0;
{
       register char *np,*p,*mark;
       int sign,esign,exp,power,dppos,sciflag;
       double *conptr;
       int ldigit,chopflag,roundflag=1;
       double n;

       n = *np0;
       /* scale into range 1.0 <= n < 10.0 */

       np = &n;        /* so we can get at the exponent */

       /* remove bias and test for neg */
       if(np[7]==0) {
               dppos=sign=esign=0;
               goto choose;
       }
       else if((exp = (np[7] & 0xff) - 0x80) < 0) {
               exp = -exp;     /* reverse sign */
               esign = 1;      /* flag negative */
       }
       else esign = 0;         /* flag positive */

       /* find power of ten for scaling */
       power = ((exp * 78) >> 8);     /* == int((exp * 78)/256) */

       /* preset decimal point */
       dppos = (esign ? -power : power) + 1;

       if (*np < 0) { /* same as if (n < 0) */
               *np &= 0x7f;   /* same as  n = -n; */
               sign=1;
       }
       else sign=0;

#ifdef DEBUG
       printf("before scale\n");
       dumpf(&n);
#endif

       /* perform gross scale */
       n = scale(n,power,esign);

#ifdef DEBUG
       printf("after scale (ddpos=%d)\n",dppos);
       dumpf(&n);
#endif

       /* finish off */
       while ( n < 1. ) {
               n *= 10;        /* multiply up */
#ifdef DEBUG
          printf("after mul up\n");
          dumpf(&n);
#endif
               dppos--;        /* dec count */
       }
       while ( n >= 10. ) {
               n /= 10;        /* divide down */
#ifdef DEBUG
          printf("after div dn\n");
          dumpf(&n);
#endif
               dppos++;        /* bump count */
       }

#ifdef DEBUG
       printf("after ranging\n");
       dumpf(&n);
#endif

       /* now 1.0 <= n < 10.0 , down to business */

choose: p = buf;
        *p++ = '0';             /* preset for rounding */
        if(sign)
                *p++ = '-';
        if(prec > 16)
                prec = 16;      /* get precision in sensible range */
        else if(prec < 0)
                prec = 0;

        chopflag = 0;
        if(format == 0) {         /* select shortest */
                chopflag = 1;             /* trailing zeroes chopped */
                if(dppos > 5)
                        goto dosci;
                else
                        goto dof;
        }
        else if(format < 0 ) {          /* scientific */
        dosci:  sciflag = 1;
                ldigit = 1;
                if(n == 0)
                        dppos = 1;      /* make positive exponent */
        }
        else {                          /* f format */
        dof:    sciflag = 0;
                if((ldigit = dppos) < 0){
                        if(ldigit + prec >= 0)
                                prec += ldigit;
                        else {
                                ldigit = -prec;
                                prec = 0;
                                roundflag = 0;
                        }
                }
                else if(ldigit + prec > 25)
                        goto dosci;
        }

        conptr = ftoatbl;

        _abnorm(&n); /* de-normalize number */
#ifdef DEBUG
        printf("after de-normalizing\n");
        dumpf(&n);
#endif

        if(ldigit < 0) {
                *p++ = '0';
                mark = p;
                *p++ = '.';
                while(ldigit++)
                        *p++ = '0';
        }
        else {
                if(ldigit == 0)
                        *p++ = '0';
                while (ldigit--) {
                        *p++ = _outdig(&n,&conptr);
#ifdef DEBUG
                        printf("after outdig(1)\n");
                        dumpf(&n);
#endif
                }

                mark = p;               /* mark for chop */
                if(prec)
                        *p++ = '.';     /* no dp if no digits after */
        }

        while (prec-->0) {
                *p++ = _outdig(&n,&conptr);
#ifdef DEBUG
                printf("after outdig(2)\n");
                dumpf(&n);
#endif
        }

#ifdef DEBUG
     *p='\0';
     printf("\n buffer is %s\n",buf);
#endif

        if(roundflag) {       /* round off */
                int carry;
                char *p1;

                *(p1 = p) = _outdig(&n,&conptr); /* put in another digit */

                for(carry = 5; ; p1--) {
                        switch (*p1) {
                                case '.':
                                        p1--;
                                        break;
                                case '-':
                                        p1[-1] = '-';
                                        *p1 = '0';
                        }

                        if(carry = ((*p1 += carry) > '9'))
                                *p1 -= 10;
                        else
                                break;
                }
        }

        if(sciflag) {
                *p++ = c ? 'E' : 'e';
                if (--dppos <0 ) {
                        dppos = -dppos;
                        *p++ = '-';
                }
                else
                        *p++ = '+';

                *p++ = (dppos/10)+'0';
                *p++ = (dppos%10)+'0';
        }
        else if(chopflag && p != mark) {      /* remove trailing zeroes & dp */
                while(--p != mark){
                        if (*p != '0') {
                                ++p;
                                break;
                        }
                }
        }

        *p = '\0';
        if(p >= &buf[30]) {
                fprintf(stderr,"pffinit buffer overflow\n");
                exit(1);
        }

        return (buf[0] == '0' ? &buf[1] : buf);
}


/*
static outdig(np,conptr)
register double *np,**conptr;
{
       int count;
       double con;

       count = 0;
       if (*conptr != ftblend) {
               con = **conptr;
               while ( *np >= con) {
                       count++;
                       *np -= con;
               }
               (*conptr)++;
       }
     return count+'0';
}
*/
#ifdef CUTOUT
double ftoatbl[] = {
     1.0e,
     1.0e-1,
     1.0e-2,
     1.0e-3,
     1.0e-4,
     1.0e-5,
     1.0e-6,
     1.0e-7,
     1.0e-8,
     1.0e-9,
     1.0e-10,
     1.0e-11,
     1.0e-12,
     1.0e-13,
     1.0e-14,
     1.0e-15,
     1.0e-16,
     1.0e-17 };
double *ftblend = ftoatbl + (sizeof ftoatbl)/(sizeof(double));
#endif
#asm
 vsect
ftoatbl
 FCB $00,$00,$00,$00,$00,$00,$00,$81 1E+00
 FCB $4C,$CC,$CC,$CC,$CC,$CC,$CD,$7D 1E-01
 FCB $23,$D7,$0A,$3D,$70,$A3,$D7,$7A 1E-02
 FCB $03,$12,$6E,$97,$8D,$4F,$DF,$77 1E-03
 FCB $51,$B7,$17,$58,$E2,$19,$65,$73 1E-04
 FCB $27,$C5,$AC,$47,$1B,$47,$84,$70 1E-05
 FCB $06,$37,$BD,$05,$AF,$6C,$6A,$6D 1E-06
 FCB $56,$BF,$94,$D5,$E5,$7A,$43,$69 1E-07
 FCB $2B,$CC,$77,$11,$84,$61,$CF,$66 1E-08
 FCB $09,$70,$5F,$41,$36,$B4,$A6,$63 1E-09
 FCB $5B,$E6,$FE,$CE,$BD,$ED,$D6,$5F 1E-10
 FCB $2F,$EB,$FF,$0B,$CB,$24,$AB,$5C 1E-11
 FCB $0C,$BC,$CC,$09,$6F,$50,$89,$59 1E-12
 FCB $61,$2E,$13,$42,$4B,$B4,$0E,$55 1E-13
 FCB $34,$24,$DC,$35,$09,$5C,$D8,$52 1E-14
 FCB $10,$1D,$7C,$F7,$3A,$B0,$AD,$4F 1E-15
 FCB $66,$95,$94,$BE,$C4,$4D,$E1,$4B 1E-16
 FCB $38,$77,$AA,$32,$36,$A4,$B4,$48 1E-17
ftblend fdb ftoatbl+144 END OF TABLE
 endsect
#endasm

#ifdef DEBUG
dumpf(s)
register char *s;
{
     int i;

     for(i=0; i<8; ++i)
          printf("%02x ",*s++ & 0xff);
     printf("\n");
}
#endif
#asm
 nam Float-ASCII Conversion Support
 ttl De-normalize floating point number

 vsect dp
bytcnt rmb 1 significant byte count
 endsect

*
*    abnorm
*         in:  2,s  - ptr to double
*         out: d    - 0
*
_abnorm pshs u save register
 ldx 4,s get float ptr
 lda 7,x get exponent
 suba #128 remove bias
 bcs abn.d branch if out of range
 ldb 0,x set implied bit
 orb #$80
 stb 0,x
 clr 7,x
 suba #4 need shifting?
 beq abn.b branch if not
abn.a lsr 0,x shift mantissa
 ror 1,x
 ror 2,x
 ror 3,x
 ror 4,x
 ror 5,x
 ror 6,x
 ror 7,x
 inca count shift
 bne abn.a branch if more
abn.b lda #8 get max bytes
abn.c deca count bytes
 bmi abn.d branch if zero
 ldb a,x is byte significant?
 beq abn.c branch if not
abn.d sta <bytcnt save for 'outdig'
 clra
 clrb
 puls u,pc
 spc 5
 ttl Output next digit
 page
*
*    outdig
*         in:  2,s  - ptr to abnormal float
*         out: d    - next digit
*
_outdig ldx 2,s get float ptr
 clra
 ldb 0,x get next digit
 lsrb
 lsrb
 lsrb
 lsrb
 addb #'0
 pshs d,u save registers
 ldb 0,x clear new digit
 andb #$0F
 stb 0,x
 bsr shift get float * 2
 lda <bytcnt get byte count
 bmi outd.e branch if none
outd.a ldb a,x get next byte
 bne outd.b branch if significant
 deca count byte
 bpl outd.a branch if more
outd.b sta <bytcnt update byte count
 bmi outd.e
 leas -8,s get scratch
outd.c ldb a,x copy float * 2
 stb a,s
 deca count byte
 bpl outd.c branch if more
 bsr shift get float * 4
 bsr shift get float * 8
 lda <bytcnt get byte count
 clrb clear carry
outd.d ldb a,x
 adcb a,s
 stb a,x
 deca count byte
 bpl outd.d branch if more
 leas 8,s return scratch
outd.e puls d,u,pc

shift lda <bytcnt
 bmi shift.c
 asl a,x
 bra shift.b
shift.a rol a,x
shift.b deca count byte
 bpl shift.a
shift.c rts
#endasm
