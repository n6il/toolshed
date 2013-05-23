/*
This C subroutine will get a CRT name from the system's
'ttytype' file, given in the name of the line. The name MUST
start with a slash. 

Needs to be rewritten in low level I/O for speed

*/

#include <stdio.h>
#include "ttydefs.h"
#include <ctype.h>

#define LINESIZE 80

char *crtbynam(line, buffer)
char *line, *buffer;

   {
   int found = 0;                      /* indicates tty line was found */

   char linebuff[LINESIZE], 
          *workptr;

   FILE * ttysfd;

/* Open the TTYTYPE file, in either of three directories */

   if ((ttysfd = fopen("/r0/sys/ttytype","r")) == NULL)
      if ((ttysfd = fopen("/r/sys/ttytype","r")) == NULL)
         ttysfd = fopen("/dd/sys/ttytype","r");

/* Search through the TTYTYPE file for a match on this 'line' */

   if (ttysfd != NULL)
      {

#ifdef DEBUG
     fprintf(stderr,"File 'ttytype' was found and opened ok\n");
#endif

      while (fgets(linebuff, LINESIZE, ttysfd) != NULL)

         {
         if (workptr = index(linebuff, ' '))
            {
            *workptr++ = '\0';              /* Terminate the LINE name */
            workptr = skipbl(workptr);          /* Index forward to the CRT name */

            if (strucmp(line, linebuff) == 0)
               {
               found++;
#ifdef DEBUG
     fprintf(stderr,"line %s was found in the file\n",line);
#endif

               strcpy(buffer, workptr);
               for (workptr = buffer; (!isspace(*workptr)); workptr++)
                  ;                                  /* null statement */
               *workptr = '\0';
#ifdef DEBUG
     fprintf(stderr,"content of buffer [crtname] is: %s\n",buffer);
#endif

               break;
               }
            }
         }

      fclose(ttysfd);

      if (found)
         return (buffer);
      else
         return ((char *)NULL);

      }
   else
      return ((char *)NULL);
   }

