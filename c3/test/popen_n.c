/*#define MAIN   /* Only use when testing routines */

#define MAIN

#ifdef MAIN
#define DEBUG
#endif

/*
 * Popen, and Pclose, for OS-9.
 *
 * Simmule Turner - simmy@nu.cs.fsu.edu - 70651,67
 *
 * V 1.3  10/22/88 - Forgot to close pipe on an error.
 * V 1.2  06/28/88 - Removed shell as parent process.
 *                 - It forks command directly now.
 * V 1.1  06/28/88 - Uses a shell to run child process SrT
 *                 - Fixed bug found by PWL, SrT
 *                 - Improved error checking, cleaned up code.
 * V 1.0  06/25/88 - Initial coding SrT
 *
 */

#include <stdio.h>

#define ERR      (-1)

#ifdef MAIN

#define LINSIZ 200
main() {
    char line[LINSIZ];
    FILE *popen(), *fp;
    int status;

    /* Test the read side of popen
     * SrT 06/25/88 */

    if (( fp = popen("procs e","r")) != NULL) {
        while (fgets(line,LINSIZ,fp) != NULL)
            printf("%s",line);
        if ((status = pclose(fp)) == ERR) {
            fprintf(stderr,"***ERR: closing read pipe ERR #%03d\n",
                            status&0xff);
            exit(1);
        }
        printf("Read status =%d\n",status&0xff);
    }
    else {
        fprintf(stderr,"***ERR: opening read pipe\n");
        exit(1);
    }

    /* Test the write side of popen
     * SrT 06/25/88 */
    if (( fp = popen("echo one two three four","w")) != NULL) {
        while (fgets(line,LINSIZ,stdin) != NULL)
            fprintf(fp,"%s",line);
        if ((status = pclose(fp)) == ERR) {
            fprintf(stderr,"***ERR: closing write pipe ERR #%03d\n",
                            status&0xff);
            exit(1);
        }
       printf("Write status =%d\n",status&0xff);
    }
    else {
        fprintf(stderr,"***ERR: opening write pipe\n");
        exit(1);
    }
}
#endif
