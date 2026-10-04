# libdefs.sed - rewrite C-library assembly-time constants (the stdio FILE struct
# field offsets and flags, from defs/stdio.a) to literals before the c3lw.awk
# filter.  These are compile-time constants, not link symbols: lwlink does not
# resolve absolute-equate exports across objects, so they must be substituted
# in each file (the same approach os9defs.sed uses for syscall codes).
#
# Longer names first so a prefix never wins over the full name (_WRITTEN before
# _WRITE).  Offsets/values are from defs/stdio.a (csect: _ptr 0,_base 2,_end 4,
# _flag 6,_fd 8,_save 10,_bufsiz 11,_iobsiz 13).

# ---- FILE struct field offsets ----
s/_bufsiz/11/g
s/_iobsiz/13/g
s/_base/2/g
s/_flag/6/g
s/_save/10/g
s/_ptr/0/g
s/_end/4/g
s/_fd/8/g

# ---- _flag bit values ----
s/_DEVMASK/\$c0/g
s/_WRITTEN/\$01/g
s/_BIGBUF/\$08/g
s/_UNBUF/\$04/g
s/_WRITE/\$02/g
s/_READ/\$01/g
s/_INIT/\$80/g
s/_EOF/\$10/g
s/_ERR/\$20/g
s/_SCF/\$40/g
s/_RBF/\$80/g

# ---- misc constants ----
s/BUFSIZ/256/g
s/_NFILE/16/g
