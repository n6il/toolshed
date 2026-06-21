/*
 * ilemit.c - c3comp1 (front end) IL emission helpers.
 *
 * Serialises parsed expression trees into the intermediate language for the
 * back end (c3comp2).  Pass-1 only.  Uses the ilw_* codec (ilwrite.c) and knows
 * the expnode layout (cj.h), which the raw codec deliberately does not.
 */

#include "cj.h"
#include "il.h"

/*
 * Emit an expression tree in POSTFIX order: children first, then the node.
 * Leaf nodes (NAME/CONST) carry the type info the back end needs (it has no
 * symbol table); interior operators carry just the op - their operands are
 * already on c3comp2's rebuild stack.
 */
ilexpr(node)
register expnode *node;
{
    if (node == NULL) {                 /* NULL child of a binary node */
        ilwrec(IL_NULL);
        ilweol();
        return;
    }

    switch (node->op) {
        case NAME: {
            register symnode *s = node->val.sp;
            register dimnode *d;
            int nd, isst;

            /* a struct/union variable carries, in its `size` field, a POINTER to
               the tag symnode (chtype completes the type from it).  That pointer
               is pass-1-only, so send the tag's type+size instead and let pass 2
               rebuild a tag symnode (pass 1 never runs chtype, so the symnode is
               always still in this unresolved form here). */
            isst = (btype(s->type) == USTRUCT);
            ilwrec(NAME);
            /* K&R: an undeclared parameter/local defaults to int.  Leave other
               UNDECL names (e.g. an undeclared called function) as UNDECL so
               pass 2's chtype can function-type them / report them. */
            ilwint((s->type == UNDECL
                     && (s->storage == AUTO || s->storage == ARG))
                    ? INT : s->type);
            ilwint(isst ? 0 : (int) s->size);  /* struct: tag ptr sent below */
            ilwint(s->storage);
            ilwint(s->offset);
            ilwint(node->modifier);
            ilwname(s->sname);
            /* the dimension chain is only meaningful (and only a valid dimnode
               list) for arrays; for structs/pointers dimptr holds other info */
            nd = 0;
            if (isary(s->type))
                for (d = s->dimptr; d; d = d->dptr) nd++;
            ilwint(nd);
            if (isary(s->type))
                for (d = s->dimptr; d; d = d->dptr) ilwint(d->dim);
            if (isst) {                         /* struct tag's type + byte size */
                register symnode *tag = (symnode *) s->size;
                ilwint(tag->type);
                ilwint((int) tag->size);
            }
            {   /* a function's ANSI-prototype parameter types, so pass 2 can
                   coerce this call's arguments exactly as single-pass does
                   (pass 2 has no headers, so the types must travel in the IL).
                   0 = unprototyped (K&R) -> no coercion.  Only the type code is
                   needed: coerceargs/cvt use nothing else from the param. */
                register symnode *a;
                int na;

                na = 0;
                for (a = s->args; a; a = a->snext) na++;
                ilwint(na);
                for (a = s->args; a; a = a->snext) ilwint(a->type);
            }
            ilweol();
            return;
        }
        case CONST:
            ilwrec(CONST);
            ilwint(node->type);
            ilwint(node->val.num);
            ilweol();
            return;
        case STRING:
            ilwrec(STRING);
            ilwint(node->val.num);     /* string temp-id (pass 2 maps to label) */
            ilweol();
            return;
        case LCONST:                    /* 32-bit value as hi,lo halves */
            ilwrec(LCONST);
            ilwint(node->type);
            ilwint((int) ((*node->val.lp >> 16) & 0xffff));
            ilwint((int) (*node->val.lp & 0xffff));
            ilweol();
            return;
#ifdef DOFLOATS
        case FCONST: {                  /* the host double's 8 raw bytes */
            register char *p = (char *) node->val.dp;
            register int i;

            ilwrec(FCONST);
            ilwint(node->type);
            for (i = 0; i < DOUBLESIZE; i++) ilwint(p[i] & 0xff);
            ilweol();
            return;
        }
#endif
        case CAST: {                    /* carries its own target type/size/dims
                                           (optim cannot recompute a cast) */
            register dimnode *d;
            int nd;

            ilexpr(node->left);         /* the operand */
            ilexpr(node->right);        /* NULL placeholder (arity-2 shape) */
            ilwrec(CAST);
            ilwint(node->type);
            ilwint((int) node->size);
            nd = 0;
            for (d = node->dimptr; d; d = d->dptr) nd++;
            ilwint(nd);
            for (d = node->dimptr; d; d = d->dptr) ilwint(d->dim);
            ilweol();
            return;
        }
        default: {
            register int a = arity(node->op);

            if (a >= 1) ilexpr(node->left);
            if (a >= 2) ilexpr(node->right);
            ilwrec(node->op);
            ilweol();
            return;
        }
    }
}
