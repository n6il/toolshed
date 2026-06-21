# c3 lwasm/lwlink back end

A modern assembler/linker back end for the c3 compiler, replacing Microware's
`rma` (Relocating Macro Assembler) and `rlink` (Relocating Linker) with the
open-source [lwtools](https://lwtools.ca) `lwasm` + `lwlink`. It produces
loadable OS-9/6809 program modules from c3 output on a modern host.

c3 itself is unchanged: it still emits its native RMA-syntax, ROF-targeted
assembly (so the original `rma`/`rlink` path, and a future 6809 self-host,
still work). This back end is a post-pass that translates that assembly to
lwasm syntax and links it as an OS-9 module.

## Pipeline

```
c3prep | c3comp | c3optx | c3lw.awk | lwasm --format=obj | lwlink --format=os9
```

## Files

| File | Role |
|------|------|
| `c3lw.awk` | Translate c3 (RMA/OS-9) assembly → lwasm syntax (the post-pass filter) |
| `os9c3.ld` | lwlink script: the built-in `os9` layout + the `irec`/`ireloc`/`icreloc` init-data sections and the `stack` reservation |
| `crt0.s`   | C startup for the lwlink model: entry/Y setup, data-area zeroing, init-image copy + pointer relocation, `_stkcheck`/`_rpterr` stubs, runtime globals |
| `buildrt.sh` | Build the runtime: `crt0.o` + `libc3lw.a` (the C helper routines via the same filter) |
| `c3lw`     | Driver script: `.c` → `.mod` (or `-S`/`-c` to stop at asm/object) |
| `buildlibc.sh` | Build the C library (`libc3.a`: stdio, strings, syscalls, memory) from `lib/lib.a`, `lib/sys.a` and the standalones here |
| `fpwrap.a` | The float/double helpers the compiler calls, over the MC6839 IEEE package (FPO9) |
| `fpinit.a` / `fpdummy.a` | `_fpinit`/`_fpterm`: link FPO9 at startup / no-op default for programs without FP |
| `MC6839.md` | MC6839 linkage design and ABI |
| `fpbench/` | Checks every `fpwrap.a` helper against the real MC6839 ROM on `usim09batch` |

## Usage

Two front ends drive the same back end:

### `c3lw` — standalone driver (simplest)

```sh
sh buildrt.sh                 # once: build crt0.o + libc3lw.a
./c3lw hello.c                # -> hello.mod  (OS-9 program module)
./c3lw -S hello.c             # -> hello.s    (lwasm assembly)
./c3lw -c hello.c             # -> hello.o    (lwobj object)
```

### The `c3` driver — full end-to-end via lwtools

The `c3` executive drives `prep | comp | optx | assemble | link`. On a unix
host it defaults its assembler/linker to the back-end shims `c3-as`/`c3-ld`
(in place of Microware `rma`/`rlink`), and its optimiser to `c3optx`. So with
the tools on `PATH` and the runtime built:

```sh
sh buildrt.sh
PATH=.../c3/build/unix/exec:.../prep:.../comp:.../optx:.../c3/lwbackend:$PATH
export C3BASE=.../c3            # where prep finds defs/ headers
c3 hello.c                      # -> hello   (OS-9 program module)
```

The tool names are overridable: `C3_OPT`, `C3_ASM`, `C3_LINK` (e.g. set them
back to `rma`/`rlink` on OS-9). `c3-as`/`c3-ld` locate the runtime relative to
themselves (resolving symlinks); override with `C3LW_HOME`.

`#include` headers live under `$(C3BASE)/defs`. `make install` puts them at
`/opt/c3/defs` (the default `C3BASE`), so an installed `c3` resolves includes
with no environment set. Header paths up to 255 chars are fine (comp's
`filename` buffer is `LINESIZE`); longer would overflow it.

## What the filter does

* `psect NAME,…` → `section code`; `endsect` → return to the enclosing section
  (RMA nests; lwasm doesn't) or `endsection` for the final one; `nam`/`ttl` →
  comments.
* Emits `EXPORT` for every non-temporary defined label and `EXTERN` for every
  referenced-but-undefined symbol (RMA auto-externed via ROF; lwasm needs them
  declared). The `_N`/`_ON` compiler temporaries and string labels stay local.
* `vsect` (static/global data) is split for the OS-9 module model, where the
  data area is *allocated* (not loaded) by `F$Fork`:
  * **uninitialised** vsects (only `rmb`/`rzb`) reserve straight into `bss`;
    `crt0` zeroes the whole data area, so they read back as 0 for free.
  * **initialised** vsects emit a loaded *image* of the initial bytes in the
    `data` section, a copy record (`src`/`end`/`dest`) in `irec`, and an equate
    per symbol giving its data-area offset (derived from the image layout, so
    `lwasm` computes the sizes — the filter doesn't). Pointer initialisers get
    a relocation entry: into the data area → `ireloc` (`crt0` adds the data
    base `Y`); into the module body, e.g. a string literal or function address
    → `icreloc` (`crt0` adds the module base). `crt0` replays all three at
    startup. See `os9c3.ld` (the matching lwlink script that carries the
    `irec`/`ireloc`/`icreloc` sections and the runtime stack).
* OS-9 module framing comes from `crt0.s` (`__start` entry) + `os9c3.ld`;
  `lwlink` sizes the data area from the `bss` sections and embeds the header.

## Status

Working end to end (verified by inspecting the emitted module header):

* functions, recursion, locals, pointers, control flow;
* string literals (emitted in the code section);
* **uninitialised** globals (int and long) — zeroed by `crt0` (`F$Fork` does
  not zero allocated memory, so this is required, not incidental);
* **initialised** globals and aggregates (`int x = 42;`, `int t[] = {…};`,
  `char s[] = "hi";`) — copied into the data area at startup;
* **pointer initialisers** — `char *p = arr;` (data pointer, relocated by the
  data base) and `char *kw[] = {"if",…};` (string-literal pointers, relocated
  by the module base);
* integer `* / % << >>` (pulls `ccmult`/`ccdiv`/`ccmod`/`cshifts` from the lib);
* long arithmetic (pulls the `cl*` helpers; `_flacc`/`_lbexit`/`_ltoacc` resolved);
* float/double **constants** are emitted as big-endian IEEE-754 (binary64 /
  binary32) on any host. float/double **arithmetic** is not usable yet — see
  Known limitations.

The init-data path was verified by decoding the linked module: the `data`
image, the `irec` copy record, and the `ireloc`/`icreloc` entries all carry the
expected offsets for a mixed test (`int`/array/string/`char *`-to-array/`long`
plus a string-literal pointer array).

**Runtime-verified on NitrOS-9** (USim, Pico-Thing L2): a module exercising
every init-data path — initialised `char[]`, `int[]`, a data-pointer
(`char *p=arr`, relocated by `+Y`), a string-literal pointer (`char *p="..."`,
relocated by `+module base`), and an uninitialised `int` read back as 0 —
printed all expected values (`tests/ctest.c`).

The runtime library is grown incrementally by adding more `lib/imath` (etc.)
sources to `buildrt.sh` — they translate through the same filter.

## Known limitations / next steps

* **Putting a module on an OS-9 disk: set the execute attribute.** `os9 copy`
  leaves a file `----r-wr` (no execute); OS-9 then refuses to load it
  (`Error #214 No Permission`, or `#215 Bad Path Name` when run by path). Run
  `os9 attr -e -pe disk,MODULE` after copying — this is what the NitrOS-9
  makefile does (`OS9ATTR_EXEC`) for every module it installs.
* **Stack size** is a fixed 1 KB reserved as the `stack` bss section (placed
  last, at the top of the data area, by `os9c3.ld`). Grow it there if needed;
  there is no overflow detection (`_stkcheck` is a stub).
* **`_rpterr`** (divide-by-zero / range error) is a no-op stub. A real version
  needs the OS-9 syscall defs.
* **Direct-page data** (`vsect dp`) is mapped to ordinary bss; no `setdp`/DP
  optimisation.
* **Static** functions/globals are over-exported (the filter can't see storage
  class), so the same `static` name in two modules would clash. A
  compiler-side back end (emitting `export`/`extern` directly, which the
  compiler can do precisely from its symbol table) would avoid this — the
  cleaner long-term option if this back end becomes primary.
* The embedded module name is the link output filename; pass a bare name (or
  add an `__os9` section) for a clean name.
* **Host-linked float/double programs still use `cfloats.a`.** `buildrt.sh`
  puts the Microware-format `lib/fmath/cfloats.a` helpers into `libc3lw.a`,
  and they don't match the compiler's IEEE-754 constants, so float/double
  arithmetic in a program linked by `c3lw`/`c3-ld` gives wrong results. The
  IEEE replacement, `fpwrap.a` over the MC6839 package, is done and verified
  (`fpbench/`, including an end-to-end smoke test of real compiler output),
  but neither the host link nor the on-target library (`mklibs`, which ships
  only the no-op `fpdummy.a`) uses it yet. Integer and long programs are
  unaffected.
* The c3 `csect` (frame/DP work area) is handled: its `name rmb N` entries
  become offset equates (the storage is stack-reserved at run time, accessed
  `name,u`). cfloats relies on this.
* The C library is ported as `libc3.a` (`buildlibc.sh`), which the
  `build/os9` cross-build links. Modules that don't assemble through the filter
  are reported by `buildlibc.sh` and left out.
