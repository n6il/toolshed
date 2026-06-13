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
| `crt0.s`   | Minimal C startup for the lwlink model (entry, Y setup, `_stkcheck`/`_rpterr` stubs, runtime globals, stack) |
| `buildrt.sh` | Build the runtime: `crt0.o` + `libc3lw.a` (the C helper routines via the same filter) |
| `c3lw`     | Driver script: `.c` → `.mod` (or `-S`/`-c` to stop at asm/object) |

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

* `psect NAME,…` → `section code`; `vsect`[`dp`] → `section bss`;
  `endsect` → return to the enclosing section (RMA nests; lwasm doesn't) or
  `endsection` for the final one; `nam`/`ttl` → comments.
* Emits `EXPORT` for every non-temporary defined label and `EXTERN` for every
  referenced-but-undefined symbol (RMA auto-externed via ROF; lwasm needs them
  declared). The `_N`/`_ON` compiler temporaries and string labels stay local.
* OS-9 module framing comes from `crt0.s` (`__start` entry) + lwlink defaults;
  `lwlink` sizes the data area from the `bss` sections and embeds the header.

## Status

Working end to end (verified by inspecting the emitted module header):

* functions, recursion, locals, pointers, control flow;
* string literals (emitted in the code section);
* **uninitialised** globals (int and long);
* integer `* / % << >>` (pulls `ccmult`/`ccdiv`/`ccmod`/`cshifts` from the lib);
* long arithmetic (pulls the `cl*` helpers; `_flacc`/`_lbexit`/`_ltoacc` resolved).

The runtime library is grown incrementally by adding more `lib/imath` (etc.)
sources to `buildrt.sh` — they translate through the same filter.

## Known limitations / next steps

* **Initialised globals** (`int x = 42;`) link but are **not** initialised at
  runtime: their values land in the `bss`-named data section, whose contents
  lwlink's OS-9 target discards (it uses only the size). The filter warns. The
  fix is the Microware approach: emit an init-data image in the code section
  and have `crt0` copy it into the data area at startup (the original
  `cstart.a` did this by reading a count+image from `etext`).
* **`_rpterr`** (divide-by-zero / range error) is a no-op stub, and `_stkcheck`
  does no overflow detection. Real versions need the OS-9 syscall defs.
* **Direct-page data** (`vsect dp`) is mapped to ordinary bss; no `setdp`/DP
  optimisation.
* **Static** functions/globals are over-exported (the filter can't see storage
  class), so the same `static` name in two modules would clash. A
  compiler-side back end (emitting `export`/`extern` directly, which the
  compiler can do precisely from its symbol table) would avoid this — the
  cleaner long-term option if this back end becomes primary.
* The embedded module name is the link output filename; pass a bare name (or
  add an `__os9` section) for a clean name.
* `float`/`double` helpers (`_dadd`…) and the C library proper (`printf`,
  syscalls, `lib.a`/`sys.a`) are not yet ported.
