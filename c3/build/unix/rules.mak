# C-Cubed: The CoCo C Compiler
# Project-Wide Rules

# This is K&R C.  Modern clang/gcc default to C2x, where the K&R idioms below
# are hard errors (or noisy warnings).  Compile as C89 and SUPPRESS the
# warnings that are inherent to the K&R style we are deliberately keeping --
# functions used before declaration, implicit int, missing/implicit returns,
# assignment-in-condition, unused locals, etc.  These are not bugs; silencing
# them gives a clean build without de-K&R-ing the sources.
#
# The pointer<->int *conversion* diagnostics are deliberately NOT suppressed
# (only demoted from error to warning): on a 64-bit host they flag real
# truncation bugs, so leaving them visible catches regressions.  They are
# currently all fixed (zero warnings).
KRFLAGS		= -std=c89 \
		  -Wno-implicit-function-declaration \
		  -Wno-implicit-int \
		  -Wno-return-type \
		  -Wno-return-mismatch \
		  -Wno-parentheses \
		  -Wno-deprecated-non-prototype \
		  -Wno-deprecated-declarations \
		  -Wno-unused-variable \
		  -Wno-unused-function \
		  -Wno-unused-label \
		  -Wno-unused-but-set-variable \
		  -Wno-unused-value \
		  -Wno-char-subscripts \
		  -Wno-constant-conversion \
		  -Wno-bitwise-instead-of-logical \
		  -Wno-comment \
		  -Wno-incompatible-function-pointer-types \
		  -Wno-format \
		  -Wno-format-security \
		  -Wno-error=int-conversion \
		  -Wno-error=int-to-pointer-cast \
		  -Wno-error=pointer-to-int-cast \
		  -Wno-error=incompatible-pointer-types

CFLAGS		= -Dunix -DUNIX -I. -g -I../../../include -Wall $(KRFLAGS)

