typedef struct {
     short type; long size; char *p; short op; long num;
     struct ex *left,*right,*up;
     short lno; char *pnt; short sux; long modifier;
} expnode;

#if 0
tranrel(op)
short op;
{
    expnode *lhs,*rhs;

    if ( lhs->sux ) {
        { expnode *t;
          rhs = t;
        }
        op = revrel(op);
    }
}
#endif
