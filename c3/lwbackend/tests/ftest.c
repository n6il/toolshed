/* ftest.c - runtime test for the float/double support.
 *
 * Constants are IEEE-754; arithmetic, compares and conversions go through the
 * MC6839 wrappers (../fpwrap.a).  Link with fpinit.o (the FP opt-in) and have
 * the FPO9 module resident or in /DD/Modules.
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

    a = 3.0; b = 2.0;
    d = a + b;  if (d == 5.0) putstr("dadd ok\n"); else putstr("dadd BAD\n");
    d = a / b;  if (d == 1.5) putstr("ddiv ok\n"); else putstr("ddiv BAD\n");
}
