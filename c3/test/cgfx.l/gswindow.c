/* status.c - testing cgfx status functions */

#include <stdio.h>
#include <sgstat.h>
#include <cgfx/window.h>
#include <cgfx/buffs.h>
#define STDIN 0
#define STDOUT 1
#define BLACK 2

main()
{
    WINDOW *w;
    w = getwind(STDOUT);

    DWEnd(STDOUT);
    DWSet(STDOUT,8,0,0,40,24,0,1,2);
    system( "merge /dd/sys/stdfonts" );
    Font(STDOUT,GRP_FONT,FNT_S8X8);
    CurOff(STDOUT);
    BColor(STDOUT,0);
    FColor(STDOUT,1);
    Clear(STDOUT);
    CurXY(STDOUT,6,10);
    write(STDOUT,"This is now a Type 8 window!",28);
    tsleep(300);
    Clear(STDOUT);
    CurXY(STDOUT,8,9);
    write(STDOUT,"I will now reset back to",24);
    CurXY(STDOUT,8,11);
    write(STDOUT,"original characteristics",24);
    tsleep(180);

    if(w != NULL)
        setwind(STDOUT,w);
}
