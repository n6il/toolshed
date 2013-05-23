/*
    setwind.c

    Sets window characteristics from a data structure previously
    loaded with the getwind() function.

    Copyright (c) 1991
    by Zack C. Sessions
*/

#include <stdio.h>
#include <sgstat.h>
#include <cgfx/window.h>

int setwind(path,window)
 int path;
 WINDOW *window;
{
    register i;

/*
    If the window structure pointer is not valid, simply return
    to the caller, with an error condition.
*/

    if(window == NULL)
        return(-1);

/*
    First set the window type by ending the current window and
    setting it back.
*/

    DWEnd(path);
    DWSet(path,window->type,0,0,window->columns,window->rows,
         window->foreground,window->background,window->border);

/*
    Now set the palette registers.
*/

/*    FColor(path,window->foreground);
    BColor(path,window->background);
    Border(path,window->border); */
    for(i = 0; i<16; ++i)
        Palette(path,i,window->palette[i]);

/*
    Lastly restore the window options information and free up the
    dynamic memory used by the WINDOW structure.
*/

    _ss_opt(path,&(window->options));

    free(window);
    return(0);
}
