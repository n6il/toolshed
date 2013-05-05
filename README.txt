
ToolShed is a package of utilities to perform cross-development from Windows,
Linux or Mac OS X computers to the Tandy Color Computer and Dragon
microcomputers.

https://sourceforge.net/projects/toolshed/


== HOW TO BUILD ON WINDOWS ==

The recommended build environment is MingW32 or MingW64 (http://mingw.org/)

The easiest way to install MingW is using a mingw-get-inst.*.exe from http://mingw.org/wiki/Getting_Started

Inside MingW, make sure you have "make" installed. There are several options,
but the simpler mingw-make should be good enough:
$ mingw-get install mingw-make

Enter the unpackaged toolshed directory and run:
$ make -C build/unix install CC=gcc


== HOW TO BUILD ON UNIX ==

To build cocofuse you will need to have FUSE libraries and header files
installed. On Debian-based systems:
$ sudo apt-get install libfuse-dev

Enter the unpackaged toolshed directory and run:
$ make -C build/unix install


== HOW TO BUILD hdbdos and dwdos ==

It is recommended to have lwtools installed (http://lwtools.projects.l-w.ca/).
Otherwise the included (but deprecated) mamou can still be used:
$ make -C dwdos AS="mamou -r -q"
$ make -C hdbdos AS="mamou -r -q"

