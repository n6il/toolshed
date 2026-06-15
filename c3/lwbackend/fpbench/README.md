# MC6839 FP wrapper bench suite (usim09batch)

Verifies `../fpwrap.a` against the real MC6839 ROM on a bare 6809.  Each harness
embeds `Float09.bin` and points `_fpreg` at `rom+$3D` (mimicking `fpinit.a`'s
`F$Link`), so no OS-9 is needed.

`Float09.bin` (the FPO9 module) is `ROM/Float09.bin` in the nfp09 repository:

    git clone https://github.com/markrvmurray/nfp09.git

The harnesses run on `usim09batch`, built from USim:

    git clone https://github.com/markrvmurray/usim.git

Assemble with lwasm's include path pointing at nfp09's `ROM` directory (`NFP09`
below is wherever you cloned it), then run:

    lwasm -I $NFP09/ROM --format=srec -o X.s19 X.s
    usim09batch --timeout=20000000 X.s19

- `fadd_baseline.s` — raw MC6839 FADD (no c3 wrapper): 2+3 = 5.0
- `dyad_bench.s`    — `_dadd/_dsub/_dmul/_ddiv`
- `rest_bench.s`    — `_dneg/_itod/_dtoi/_dcmpr`
- `conv_bench.s`    — `_ltod/_dtol/_dtof/_ftod/_utod`
- `incdec_bench.s`  — `_dinc/_ddec/_finc/_fdec`

End-to-end (real compiler output + fpwrap.a + FPO9), the "double smoke test":

    c3prep -DOS9 -v=../../defs dtest.c | c3comp | c3optx | awk -f ../c3lw.awk \
        | lwasm --format=obj -o dt.o -
    lwasm -I $NFP09/ROM --format=obj -o flatcrt0.o flatcrt0.s
    # fpwrap.o: filter ../fpwrap.a (see ../buildlibc.sh asma) then lwasm
    lwlink --format=srec -s flatbench.ld -o dtbench.srec flatcrt0.o dt.o fpwrap.o
    usim09batch dtbench.srec        # -> C:4014000000000000 I:0006
