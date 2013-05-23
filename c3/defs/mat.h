/*
 * mat.h - Matrix functions
 *
 * $Id: mat.h,v 1.3 2005/08/13 15:07:11 boisy Exp $
 *
 * (C) 2005 The C^3 Compiler Project
 * http://www.nitros9.org/c3/
 *
 * Notes:
 *
 * Edt/Rev  YYYY/MM/DD  Modified by
 * Comment
 * ------------------------------------------------------------------
 *          2005/08/12  Boisy G. Pitre
 * Brought in from Carl Kreider's CLIB package.
 */

#ifndef _MAT_H
#define _MAT_H

typedef struct
{
	int             m_rows,
	                m_cols;
	double          m_value[1];
}               MAT;


double          m_cofactor(), m_determinant();
MAT            *m_copy(), *m_create(), *m_invert(), *m_transpose(),
               *m_multiply(), *m_solve(), *m_add(), *m_sub(), *m_read();

#define  m_v(m, r, c)   (m->m_value[r * (m->m_cols) + c])
#define  M_NULL         ((MAT *)0)

#endif				/* _MAT_H */
