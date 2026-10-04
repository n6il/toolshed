; os9c3.ld - lwlink linker script for the c3 OS-9-module backend.
;
; This is the built-in lwlink "os9" script (see lwtools lwlink/script.c)
; extended with three extra *loaded* sections that carry the initialised-data
; machinery the c3lw.awk filter emits:
;
;   irec     - copy records:  src(2) end(2) dest(2)  per initialised data block
;   ireloc   - data-pointer relocations: data-area offset of each word that
;              holds a pointer into the data area (crt0 adds the data base Y)
;   icreloc  - code-pointer relocations: data-area offset of each word that
;              holds a pointer into the module body, e.g. a string literal or
;              function address (crt0 adds the module base)
;
; "define basesympat/lensympat" make lwlink synthesise s_<section>/l_<section>
; (base offset and length) for every named section, which crt0 imports to walk
; these tables.  The initialised *image* itself rides in the standard "data"
; section; "bss" remains the runtime data area (load 0, size only).
define basesympat s_%s
define lensympat l_%s
section code load 000D
section .text
section data
section .data
section irec
section ireloc
section icreloc
section bss,bss load 0000
section .bss,bss
; The runtime stack: a bss section listed LAST so lwlink places it at the top of
; the data area, above all statics.  crt0 reserves it (section stack); the
; descending hardware stack runs down through this gap before it could ever
; reach the Y-relative statics below.  (lwlink's os9 writer ignores "stacksize",
; so the reservation must be a real bss section, not the script directive.)
section stack,bss
entry __start
