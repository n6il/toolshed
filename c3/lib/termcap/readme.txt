
                                  OS9 Termcap                               


                              A Brief Introduction

                                 By Pete Lyall

    What is 'TERMCAP'?  Termcap is an acronym for 'terminal capabilities'.
    Its purpose is to allow programmers to write programs that take
    advantage of some of the features available on modern CRT display
    terminals, without having to know what kind of terminal will be used.
    This way, a programmer may use general purpose routines to do things
    like move the cursor, erase a line, highlight a portion of the screen,
    and so on without having to worry about what kinds of characters or
    control sequences need to be used.  This version of OS9 Termcap was
    ported to OS9/6809 by Simmule Turner, with some assistance from Pete
    Lyall.

    Termcap was originally developed and used with the UNIX operating
    system. In the later releases of Unix, it has been replaced by a more
    complex and efficient system known as 'Termlib'.  Termlib has backwards
    compatible features that allow programs written for Termcap to be used.
    As of this writing, Termlib has not been ported to OS9, but possibly
    will be in the not too distant future.

                               Who Needs Termcap?

    Users - There will be an awful lot of games and applications programs
    that can be ported from Unix or written from scratch now that the type
    of terminal to be used by the end user is not an obstacle.  In the past,
    people wanting to to fancy screen I/O, had to hard code their programs
    to work with a coco3, a VT-100, or whatever.  With termcap and its
    programmer routines available, that should no longer be a problem.  All
    a user needs to use a program written with termcap features is:

                   o - The program itself

                   o - A file called 'ttytype' (explained below)

                   o - A file called 'termcap' (explained below)


    Programmers - It is now possible to write very elegant screen-oriented
    programs with little or no concern about what sort of terminal the end
    user might be using.  Typical systems where this flexibility might be
    useful are:

        o - BBS Systems (users with a variety of terminal types)

        o - Os9 systems with local terminals attached, possibly of different
            types

        o - Those wishing to run ported, screen oriented code from other
            systems

                                 Termcap Basics

                        (Not required reading for users)

    Termcap's operation is really very simple.  A file called TERMCAP is
    kept


                                     Page 1                                 


                                  OS9 Termcap                               


    on the system.  This file contains the names of the terminals that will
    be supported, and with each name, lists that terminal's capabilities in
    a shorthand form that is understandable to the termcap routines.  These
    capabilities fall into three categories: numbers, booleans (true or
    false), and strings.  Numbers are used for things like how many rows or
    columns a terminal might have, booleans are used to indicate that a
    terminal does or doesn't have a certain characteristic (example: whether
    a terminal wraps to the next line when a character is printed in the
    rightmost column, etc.), and strings are the actual string used to cause
    a certain action to take place, like erasing to the end of the screen.
    There are unique two-character mnemonics for each capability, and there
    are routines provided in the library 'termcap.l' to allow programmers to
    easily get at these strings, numbers, and booleans using their
    mnemonics.

    The mechanics of how a termcap enhanced program work are also quite
    simple. It must simply:

    a) Look up the CRT name for the line that the user is using.  This
    information is maintained in the 'ttytype' file.  It is of the format:

    /line1_name crt_name
    /line2_name crt_name

    This file is kept in /R/SYS, /R0/SYS, or in /DD/SYS.  The routines that
    perform the lookup ('crtbynm()', and 'crtbypth()') will search all
    three directories automatically.  My 'ttytype' file looks like this:

    /term wyse50
    /t1 x10
    /t2 coco3
    /t3 coco
    /t4 wyse50
    /t5 wyse50
    /t6 hayes
    /t7 usr
    /p okidata
    /p2 juki
    /pl 6pen

    This file is easily managed by the 'ttyset' utility.  The ttyset utility
    can be set up to run automatically when each user logs in to set his/her
    entry in the 'ttytype' to the appropriate crt name (i.e.  VT-100,
    Wyse50, etc.), or it can be run manually.  Please see the ttyset
    documentation for a detailed discussion of the command and its features.

    b) Once the CRT name is found using step a) above, use the termcap
    routines to look up that terminal name in the 'termcap' file.  The
    termcap file may also be located in any of the directories /R0/SYS,
    /R/SYS, or /DD/SYS, and will be automatically searched for in that
    order. Once the file is found, its entries will be scanned looking for
    the requested terminal.  If it is found, a copy of its capabilities are
    placed into an internal memory buffer for quick access.  This is done by
    the 'tgetent()' routine (terminal get entry) that must be used prior to
    any other termcap calls.  A brief termcap file that supports Wyse50
    terminal, a coco3, an Atari ST in VT52 emulator mode, and a Radio Shack
    Model-100, and a few others is supplied in a file called 'termcap' in
    this


                                     Page 2                                 


                                  OS9 Termcap                               


    archive.

    c) Once the terminal's capabilities have been found and loaded via the
    'tgetent()' call, all the other termcap functions may be used to find
    out terminal characteristics and control the terminal.  They will
    consult the data obtained by tgetent, which is already in memory.  This
    results in very fast operation.  These basic functions will be described
    in greater detail in the file 'termcap.man', but for the curious are:

    Tgetnum - returns a number from the terminal's capabilities list (like
    number of columns, rows, etc.)

    Tgetflag - returns a TRUE or FALSE (1 or 0) indication of whether a
    terminal has a given capability or feature.

    Tgetstr - returns a string that performs a given requested function.

    Tgoto - using the terminal's cursor motion string template, and a column
    and a row number to go to, this will return a string that will send the
    cursor to the requested position.

    Tputs - send out a string with any time delays or padding that a given
    terminal might need.

                       An Example Installation for Coco3

    For a simple coco3 system, the system administrator may want to create a
    'ttytype' file in the /R0/SYS, /R/SYS, or /DD/SYS directory that looks
    something like this:

    /term coco3
    /t2 Wyse50
    /w1 coco3
    /w2 coco3
    /w3 coco3
    and so on ...

    You get the picture.  In the case of a standard coco3, the /Term might
    not be an 80 column device, but more likely is a 32 by 16 device.  You
    may want to make a variation of the 'coco3' termcap entry above that has
    lines and columns adjusted to 32 and 16 respectively.  It might be
    appropriate to call this type of terminal 'vdg' or 'cc3vdg', or
    something similar.  In fact, when you become more familiar with termcap,
    you'll find that it allows terminals that are almost alike to share
    entries, and then one terminal description simply lists the terminal it
    looks most like (using the 'tc' parameter), and then lists those things
    that are different about itself.  Perhaps 'cc3vdg' could be just a
    variation on the basic 'coco3' entry, with only lines and columns
    changed.

    If your /t2 port has a different terminal type on it, by all means enter
    whatever is appropriate.  If you have no /t2, or no terminal installed,
    then just omit that line.  If you have a variety of users with different
    terminals logging in on /t2, then use the ttyset utility to alter the
    'ttytype' file appropriately for you.

    If you have a ramdisk, you should install the 'ttytype' there in the SYS


                                     Page 3                                 



                                  OS9 Termcap                               


    directory.  If you have no SYS directory in your ramdisk, make one.  I
    strongly recommend that you use the 'mkdir' utility instead of the stock
    'makdir' utility, as the former will allow you to create a very small
    SYS directory, thus preserving ramdisk space.  An example might be
    'mkdir -e6 /R0/SYS'.  This would make a /R0/SYS directory with room for
    6 entries (and 2 more for '.' and '..'), using only 1 sector for the SYS
    directory.  This will provide much faster service to all programs that
    use the termcap routines.  Otherwise, every program that uses termcap
    will have to access the /DD/SYS directory.  This could be somewhat slow
    of floppy based systems (although only at program startup time).

    Next, you will need a termcap file that supports all the terminal types
    you are planning to service.  The small example listed above may be used
    as a starting termcap file.  Breaking down the meanings of the
    individual termcap mnemonics will be discussed in the termcap man page.
    Writing termcap entries can also be achieved after reading the man page,
    but in most cases, the terminal may already have an entry
    available.Those of us with access to unix systems can usually find
    entries for other popular (and many obscure) terminal types, and post
    them on request.  It is very likely that I will later post a large
    termcap file that can be used as a cut and paste source for individual
    termcap files.  This termcap file also needs to be placed into /R0/SYS,
    /R/SYS, or /DD/SYS.

    Voila!  That's it.  Once the 'ttytype' and 'termcap' files are in the
    proper directories, you are ready to run termcap applications.  As
    mentioned above, you will probably also want to get the 'ttyset' utility
    and possibly the 'mkdir' utility.  You could even automate the whole
    installation by putting it in your startup file, like so:

    *
    * Install termcap support
    *
    iniz /r0
    mkdir -e6 /r0/sys
    *
    * or makdir /r0/sys
    *
    copy /dd/sys/termcap /r0/sys/termcap
    copy /dd/sys/ttytype /r0/sys/ttytype
    echo Termcap support is installed
    *

                                    Summary

    I will probably be uploading a termcap "USER'S KIT" and also a termcap
    "PROGRAMMER'S KIT".  The programmer's kit will include the library
    (termcap.l), as well as the detailed breakdown of both the C language
    calls to use termcap, and what the assorted termcap mnemonics mean.  In
    addition, I will also upload a few applications or games that use
    termcap an example.  The programmer's kit will likely also include a few
    example sources.  In the very near future, look for 'curses' - a high
    level terminal and window manager suite of tools that will make porting
    and full screen development even easier.  Curses uses termcap routines
    to do its dirtywork.  Good luck with it, and may all your  screen I/O be
    portable.



                                     Page 4                                 



                                  OS9 Termcap                               


                                   Pete Lyall

                                 Simmule Turner

    






















































                                     Page 5                                 


