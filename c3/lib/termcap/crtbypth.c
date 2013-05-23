/* This C subroutine accepts a path number, determines the name
   of the device/line, and then calls crtbyname()

   Really needs to be re-written in lower level I/O for speed

*/

#include <stdio.h>
#include <os9.h>

extern char *crtbynam();

char *
crtbypth(path,buffer)

int path;
char *buffer;

{
char line[32];

     *line = '/';
     getstat(SS_DEVNM, path, &line[1]);
     strhcpy(line,line);

     return(crtbynam(line,buffer));
     
}


