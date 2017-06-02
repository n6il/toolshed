#include "cj.h"
#include <stdlib.h>
#include <string.h>

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

void preinit(void)
{
        lptr=line;
        *line='\0';
        cc=' ';
}

void blanks(void)
{
        while(cc==' ' || cc == '\t') getch();
}

void getch(void)
{
        if((cc= *lptr++))
                ;
        else
                cc=getline1();
}

char getline1(void)
{
        int lno, n, x;
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
                                fprintf(code,"%s\n",line);
                                continue;
                        case NEWFNAME:
                        		if( strlen( line ) > FNAMESIZE )
                        		{
                        			fprintf( stderr, "Path %s too long, increase size of FNAMESIZE in cj.h.\n", line );
                        			exit( 1 );
                        		}
                                strcpy(filename,line);
                                if(cgets() == NULL)
                                        return EOF;
                                continue;
                        case PSECT:
                                strcpy(temp,line);
                                if(cgets() == NULL)
                                        return EOF;
                                fprintf(code," psect %s,0,0,%d,0,0\n",
                                             temp,atoi(line));
                                fprintf(code," nam %s\n",temp);
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
                                        exit(1);
                                continue;
                        }
                }
                else {
                        ++lineno;
                        return ' ';
                }
        }
}

#ifndef UNIX
atoi(s)
register char *s;
{
        int c;
        int n = 0;

        while(isdigit(c = *s++))
                n = n * 10 + (c - '0');
        return n;
}
#endif

char *cgets()           /* Function to input a line of source.
                         * Returns a pointer to the start of the line
                         * if all is well.
                         * Returns NULL on end of file.
                         * 'exit' on error or line too long
                         */
{
        int c;
        static char *max = &line[LINESIZE-1];   /* marker for size */
        register char *lp = line;

        if((c = getc(in)) == EOF){
                if(ferror(in)) {
                        fputs("INPUT FILE ERROR : TEMPORARY FILE\n",stderr);
                        exit(1);
                }
                return NULL;
        }

        while(lp != max)
                switch (c) {
                        case '\n':
                        case EOF:
                                *lp = '\0';
                                return line;    /* normal return point */
                        default:
                                *lp++ = c;
                                c = getc(in);
                }

        /* if we get here, lp == max; i.e. too far */
        symline = ++lineno;             /* fix up for 'error' */
        symptr = line;
        fatal("input line too long");   /* NO RETURN ! */

		return NULL;
}
