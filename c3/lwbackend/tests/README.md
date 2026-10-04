# lwbackend init-data runtime test

`ctest` exercises every init-data path the lwasm/lwlink OS-9 backend has to get
right at run time (crt0 image copy, pointer relocation, bss zeroing). It was
the test that confirmed the backend works on real NitrOS-9.

| File | Role |
|------|------|
| `ctest.c` | init-data test — prints one line per init-data path |
| `ftest.c` | float/double test — constants, `int`→`double`, add and divide (see below) |
| `putstr.s` | OS-9 `I$Write` helper (`putstr(char*)`; arg in D, Y preserved) |
| `mktestdisk.sh` | compile + link a test and build a `/j0` disk image with the execute attribute set |

`mktestdisk.sh` takes the test name: `sh mktestdisk.sh` (→ `ctest`) or
`sh mktestdisk.sh ftest`.

## What it checks

`ctest` on NitrOS-9 prints exactly:

```
arr ok      initialised char array          -> copied from the loaded image
arr ok      char *parr = arr;               -> data-pointer reloc   (crt0 +Y)
lit ok      char *plit = "lit ok\n";        -> code-pointer reloc   (crt0 +module base)
tab ok      int tab[3] = {65,66,67};        -> initialised int array
zero ok     int zero;  (uninitialised)      -> crt0 zeroed the data area
```

A missing or garbled line points straight at the broken path.

`ftest` checks float/double support. The compiler now emits constants as
IEEE-754, but the linked helpers (`cfloats.a`) use Microware's float format, so
its arithmetic checks (`dadd`, `ddiv`) are not expected to pass until the
helpers are replaced by the MC6839 package — see ../README.md "Known
limitations" and ../MC6839.md.

## Build

Prerequisites: the c3 unix build (`make -C build/unix all` from `c3/`), the
runtime (`sh ../buildrt.sh` → `crt0.o` + `libc3lw.a`), `lwasm`/`lwlink`, and
Toolshed `os9`.

```sh
sh mktestdisk.sh            # -> ./ctest_j0.dsk  (module name "ctest", execute attr set)
```

## Run under USim / NitrOS-9 (Pico-Thing L2)

You need a NitrOS-9 tree with the Pico-Thing Level 2 recipe built — the boot
disk and the REL loader the emulator starts from:

```sh
git clone https://github.com/nitros9project/nitros9.git
cd nitros9/recipes/picothing/l2
NITROS9DIR=$PWD/../../.. make DSKIMAGE=NOS9_6809_L2_DEV_picothing_i0.dsk \
    OS9FORMAT_CMD="os9 format -e -l65536"
NITROS9DIR=$PWD/../../.. make rel_picothing.srec
```

and `usim09pt`, built from USim:

    git clone https://github.com/markrvmurray/usim.git

The recipe's boot only has the `I0`–`I7` IDE descriptors, so there is no `/j0`
to mount the test disk on. Copy the module onto a copy of the boot disk instead
(the `os9 attr` step is required — see the gotchas below):

```sh
cp NOS9_6809_L2_DEV_picothing_i0.dsk boot.dsk
os9 copy /path/to/ctest_j0.dsk,ctest ctest
os9 copy -r ctest boot.dsk,CMDS/ctest
os9 attr -e -pe boot.dsk,CMDS/ctest
usim09pt -d boot.dsk rel_picothing.srec
```

At the `User name?:` prompt press Enter (empty = superuser), then run `ctest`.

## Gotchas learned the hard way

* **Execute attribute is mandatory.** `os9 copy` leaves a file `----r-wr`;
  OS-9 then refuses to load the module (`Error #214 No Permission`, or
  `#215 Bad Path Name` when run by path). `mktestdisk.sh` runs
  `os9 attr -e -pe` to fix it — the same thing the NitrOS-9 makefile does
  (`OS9ATTR_EXEC`) for every module it installs. (`#216 Path Name Not Found`
  is the different "file genuinely absent" error.)
* The console is CR-terminated and the SCF driver pads CR with NULs, so when
  capturing output non-interactively, de-NUL it: `tr -d '\000' | tr '\r' '\n'`.
