# ToolShed

ToolShed is a cornucopia of tools and source code for the Tandy Color Computer and Dragon computers.

The repository contains:
- `os9` and `decb` for copying files to/from host file systems to disk images
- CoCo & Dragon system ROM source code, and source code for custom ROMs like HDB-DOS, DriveWire DOS, and SuperDOS
- Microware C compiler souce code for cross-hosted compilation (currently needs work)
- Assemblers to perform cross-development from Windows, Linux, and macOS (see the NOTE below on assembler recommendations)
- Other miscellaneous tools

**NOTE:** while the venerable 6809 cross-assembler, mamou, is part of the repository, it is only kept for historical value. Everyone should really be using William Astle's excellent LWTOOLS which contains the *lwasm* 6809 assembler and *lwlink* linker. [Download the latest version of the source here.](http://lwtools.projects.l-w.ca)

## Building on Windows

The recommended build environment is [MingW32 or MingW64](http://mingw.org/), [MSYS2](http://msys2.github.io/), or the [WSL subsystem for Windows 10+](https://en.wikipedia.org/wiki/Windows_Subsystem_for_Linux).

The easiest way to install MingW is using a mingw-get-inst.*.exe [from here](http://mingw.org/wiki/Getting_Started).

Inside MingW, make sure you have "make" installed. There are several options, but the simpler mingw-make should be good enough:
```
$ mingw-get install mingw-make
```

Enter the unpackaged toolshed directory and run:
```
$ make -C build/unix install CC=gcc
```

## Building on Linux

To build cocofuse for Linux, you need FUSE libraries and header files installed. On Debian-based systems:
```
$ sudo apt-get install libfuse-dev
```

Enter the unpackaged toolshed directory and run:
```
$ make -C build/unix install
```

## Building on macOS

To build cocofuse for the Mac, you need FUSE libraries and header files installed. 

The best way to do this is to first [visit the Homebrew page](https://brew.sh) and use the simple one-line ruby command to install Homebrew on your Mac.

Once that's done, use `brew` to install macfuse (`osxfuse` has been succeeded by `macfuse` as of version 4.0.0.):

```
brew install macfuse
```

If you have previously install osxfuse, you can use brew command to uninstall `oxsfuse` and install `macfuse`

```
brew uninstall osxfuse
brew install macfuse
```

Then, enter the unpackaged toolshed directory and run:
```
$ make -C build/unix install
```

## Building HDB-DOS and DriveWire DOS

It is highly recommended to have [LWTOOLS](http://lwtools.projects.l-w.ca/) installed. You also need `makewav` to build WAV files. See **hdbdos/README.txt** and the makefiles for different build options.

To build all default flavors:
```
$ make -C dwdos
$ make -C hdbdos
$ make -C superdos
```

Instead of LWTOOLS, you can use the deprecated `mamou`:
```
$ make -C dwdos AS="mamou -r -q"
$ make -C hdbdos AS="mamou -r -q"
```

Note that SuperDOS still builds with `mamou` by default.
