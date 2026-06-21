#include "cj.h"
#include "il.h"

#define        ESCHAR          '#'
#define        ERROR          '0'
#define        FATERR         '1'
#define        ASSLINE        '2'
#define        NEWLINO        '5'
#define        SOURCE         '6'
#define        NEWFNAME       '7'
#define        ARTHFLG        '8'
#define        PSECT          'P'

#define isdigit(c) (chartab[c] == DIGIT)
/*
#ifdef  SPLIT
static long soffset;
#endif
*/


void preinit()
{
        lptr=line;
        *line='\0';
        curch=' ';
}

void skipblanks()        /* renamed from blanks(): the c3 libc printf (printf.s)
                       exports a DIFFERENT `blanks` (field padding), and c3comp1
                       links both -> the linker bound this lexer call to printf's
                       blanks, which jumps through an uninitialised stack slot
                       (the wild-jump/hang on real L2).  The host system libc has
                       no such symbol, so the clash was invisible there. */
{
        while(curch==' ' || curch == '\t') getch();
}

void getch()
{
        if(curch= *lptr++)
                ;
        else
                curch=getline1();
}

char getline1()
{
        int lno, n,x,*ptr;
        static char temp[LINESIZE];
        static direct int lineflg;
        char *cgets();

        if(lineflg == 0) {
                lineflg = 1;
                lptr = "";
                return ' ';
        }
        lineflg = 0;
        strcpy(lastline,line);
        for(;;) {
                if((lptr=cgets())==NULL) {
                        return EOF;
                }

                if(*lptr == ESCHAR) {
                        n = lptr[1];
                        if(cgets() == NULL)
                                return EOF;
                        switch(n)  {
                        case NEWLINO:
                                lineno=atoi(line);
                                continue;
                        case SOURCE:
                                comment();
                        case ASSLINE:
#ifdef SPLIT
                                passthru();
#endif
                                fprintf(code,"%s\n",line);
                                continue;
                        case NEWFNAME:
                                strcpy(filename,line);
#ifdef  SPLIT
                                p2file(filename);
#endif
                                if(cgets() == NULL)
                                        return EOF;
                                continue;
                        case PSECT:
                                strcpy(temp,line);
                                if(cgets() == NULL)
                                        return EOF;
#ifdef MKIL
                                ilwrec(IL_PSECT);
                                ilwname(temp);
                                ilwint(atoi(line));
                                ilweol();
#else
                                fprintf(code," psect %s,0,0,%d,0,0\n",
                                             temp,atoi(line));
                                fprintf(code," nam %s\n",temp);
#endif
                                continue;
                        case ERROR:
                        case FATERR:
                                strcpy(temp,line);
                                if(cgets() == NULL)
                                        return EOF;
                                lno = atoi(line);
                                if(cgets() == NULL)
                                        return EOF;
                                x = atoi(line);
                                if(cgets() == NULL)
                                        return EOF;
                                if (lno)
                                        printf("%s : line %d ",filename,lno );
                                else
                                        printf("argument : ");
                                printf("**** %s ****\n",line);
                                if(temp[0]){
                                        puts(temp);
                                        for(;x--;)
                                                putchar(' ');
                                        puts("^");
                                }
                                if (n == FATERR)
#ifdef  SPLIT
                                        tidy();
#else
                                        exit(1);
#endif
                                continue;
                        }
                }
                else {
#ifdef  SPLIT
/*
                        p2line(++lineno,soffset);
*/
                        ++lineno;
#else
                        ++lineno;
#endif
                        return ' ';
                }
        }
}

char *cgets()           /* Function to input a line of source.
                         * Returns a pointer to the start of the line
                         * if all is well.
                         * Returns NULL on end of file.
                         * 'exit' on error or line too long
                         */
{
        int c;
        static char *max = &line[LINESIZE-1];   /* marker for size */
        static int pending = -1;        /* one-char pushback for a CR/LF pair partner */
        register char *lp = line;

#ifdef  SPLIT
#ifdef CUTOUT
        soffset = ftell(in);             /* file position for pass 2 */
#endif
#endif
        /* Read the first char, honouring a partner pushed back by the previous
           line.  Line endings are accepted as CR, LF, CRLF or LFCR regardless of
           how the file was transferred: OS-9's '\n' is CR ($0D) and the host's is
           LF ($0A), and an .i copied raw between them keeps the source ending, so
           we must terminate on EITHER byte (012/015) explicitly rather than on the
           platform '\n'.  Without this a host-LF file read on OS-9 has no line
           ends -> the whole file becomes one unterminated line -> no tokens. */
        c = (pending >= 0) ? pending : getc(in);
        pending = -1;
        if(c == EOF){
                if(ferror(in)) {
                        fputs("INPUT FILE ERROR : TEMPORARY FILE\n",stderr);
#ifdef  SPLIT
                        tidy();
#else
                        exit(1);
#endif
                }
                return NULL;
        }

        while(lp != max)
                switch (c) {
                        case 015:               /* CR */
                        case 012:               /* LF */
                                {       /* swallow the partner of a CRLF / LFCR pair
                                           so any of CR, LF, CRLF, LFCR is one line
                                           end; a lone partner is pushed back. */
                                        int c2 = getc(in);
                                        if(!(((c == 015) && (c2 == 012)) ||
                                             ((c == 012) && (c2 == 015))))
                                                pending = c2;
                                }
                                *lp = '\0';
                                return line;    /* normal return point */
                        case EOF:
                                *lp = '\0';
                                return line;
                        default:
                                *lp++ = c;
                                c = getc(in);
                }

        /* if we get here, lp == max; i.e. too far */
        symline = ++lineno;             /* fix up for 'error' */
        symptr = line;
        fatal("input line too long");   /* NO RETURN ! */
}
