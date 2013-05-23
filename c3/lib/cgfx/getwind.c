/*
    getwind.c

    Gets window characteristics and stores them away, returning
    a pointer to the structure. If any error occurs, the NULL
    pointer is returned.

    BE SURE TO SEE THE READ.ME FILE ON INSTRUCTIONS FOR COMPILING
    THIS FUNCTION!!!

    Copyright (c) 1991
    by Zack C. Sessions
*/

#include <stdio.h>
#include <sgstat.h>
#include <cgfx/window.h>

WINDOW *getwind(path)
int path;
{
    WINDOW *w;

/*
    First allocate some dynamic memory for the WINDOW data structure.
*/

    if((w = (WINDOW *) malloc(sizeof(WINDOW))) == NULL)
        return(NULL);

/*
    Now, start getting the various information about the window.
    First the window type.
*/

    if(_gs_styp(path,&(w->type)) == -1) {
        free(w);
        return(NULL);
    }

/*
    Next get the size of the window.
*/

    if(_gs_scsz(path,&(w->columns),&(w->rows)) == -1) {
        free(w);
        return(NULL);
    }

/*
    Next get the palette registers for the foreground, background,
    and border.
*/

    if(_gs_fbrg(path,&(w->foreground),&(w->background),&(w->border)) == -1) {
        free(w);
        return(NULL);
    }

/*
    Now get the contents of the current palette registers.
*/

    if(_gs_palt(path,w->palette) == -1) {
        free(w);
        return(NULL);
    }

/*
    Lastly, get the window options information.
*/

    if(_gs_opt(path,&(w->options)) != -1)
        return(w);
    else {
        free(w);
        return(NULL);
    }
}
