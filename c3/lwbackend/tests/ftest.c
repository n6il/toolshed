/* ftest.c - runtime test for the float/double support.
 *
 * The compiler emits float/double constants as big-endian IEEE-754 on any
 * host.  The helpers linked by the lwasm back end (lib/fmath/cfloats.a) use
 * Microware's float format, so the arithmetic checks below only pass once
 * those helpers are replaced by the MC6839 IEEE package (see ../MC6839.md).
 */
double a, b, d;
int    i;

main()
{
    a = 5.0;                            /* a float CONSTANT */
    if (a == 0.0) putstr("const 5.0 -> ZERO  (constant emission is broken)\n");
    else          putstr("const 5.0 -> nonzero (constants fixed!)\n");

    i = 5; a = i;                       /* _itod: int -> double, a real 5.0 */
    if (a == 0.0) putstr("itod 5 -> ZERO  (helper broken)\n");
    else          putstr("itod 5 -> nonzero (helper works)\n");

    /* meaningful once the helpers are IEEE (MC6839): */
    a = 3.0; b = 2.0;
    d = a + b;  if (d == 5.0) putstr("dadd ok\n"); else putstr("dadd BAD\n");
    d = a / b;  if (d == 1.5) putstr("ddiv ok\n"); else putstr("ddiv BAD\n");
}
