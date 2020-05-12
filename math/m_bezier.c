/* Copyright (c) Charles A. Daniels <http://cdaniels.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This module is based on code published in Graphics Gems V by Alan W. Paeth
 * (ch IV.8, pp206) and written by Rober D. Miller. Said code was Copyright (c)
 * 1995 Academic Press Inc. and distributed under the license:
 *
 * EULA: The Graphics Gems code is copyright-protected. In other words, you
 * cannot claim the text of the code as your own and resell it. Using the code
 * is permitted in any program, product, or library, non-commercial or
 * commercial. Giving credit is not required, though is a nice gesture. The
 * code comes as-is, and if there are any flaws or problems with any Gems code,
 * nobody involved with Gems - authors, editors, publishers, or webmasters -
 * are to be held responsible. Basically, don't be a jerk, and remember that
 * anything free comes with no guarantee.
 */

#include <agar/core/core.h>
#include <agar/math/m.h>

#include <agar/config/enable_gui.h>

#ifdef ENABLE_GUI
# include <agar/gui/gui.h>
#include <agar/gui/widget.h>
#include <agar/math/m_bezier_primitives.h>
#endif


/* Setup Bezier coefficient array once for each control polygon. */
void M_BezierForm2(M_PointSet2* p, M_PointSet2* c)
{
	int k;
	M_Real n, choose = 1.0;
	n = p->n - 1;

	M_PointSetAlloc2(c, p->n);

	/* XXX: should be an error condition of n < c->n */

	for(k = 0; k <= n; k++) {
		if (k == 0) choose = 1.0;
		else if (k == 1) choose = n;
		else choose = choose *(n-k+1)/k;
		c->p[k].x = p->p[k].x *choose;
		c->p[k].y = p->p[k].y *choose;
		c->n++;
	};

}

/* Setup Bezier coefficient array once for each control polygon. */
void M_BezierForm3(M_PointSet3* p, M_PointSet3* c)
{
	int k;
	M_Real n, choose = 1.0;
	n = p->n - 1;

	M_PointSetAlloc3(c, p->n);

	/* XXX: should be an error condition of n < c->n */

	for(k = 0; k <= n; k++) {
		if (k == 0) choose = 1.0;
		else if (k == 1) choose = n;
		else choose = choose *(n-k+1)/k;
		c->p[k].x = p->p[k].x *choose;
		c->p[k].y = p->p[k].y *choose;
		c->p[k].z = p->p[k].z *choose;
	};
}

/* Return Point pt(t), t <= 0 <= 1 from c, given the number of Points in
 * control polygon. BezierForm must be called once for any given control
 * polygon. */
void M_BezierCurve2(M_PointSet2* c, M_Real t, M_Vector2* pt)
{
	int k, n;
	M_Real t1, tt, u;
	M_PointSet2 b;

	M_PointSetInit2(&b);
	M_PointSetAlloc2(&b, c->n);

	/* XXX: need to do bounds checking on c */

	n = c->n - 1;
	u = t;
	b.p[0].x = c->p[0].x;
	b.p[0].y = c->p[0].y;
	for(k =1; k <=n; k++) {
		b.p[k].x = c->p[k].x *u;
		b.p[k].y = c->p[k].y *u;
		u =u*t;
	};

	pt->x = b.p[n].x;
	pt->y = b.p[n].y;
	t1 = 1-t;          tt = t1;
	for(k =n-1; k >=0; k--) {
		pt->x += b.p[k].x *tt;
		pt->y += b.p[k].y *tt;
		tt =tt*t1;
	}

	M_PointSetFree2(&b);
}

/* Return Point pt(t), t <= 0 <= 1 from c, given the number of Points in
 * control polygon. BezierForm must be called once for any given control
 * polygon. */
void M_BezierCurve3(M_PointSet3* c, M_Real t, M_Vector3* pt)
{
	int k, n;
	M_Real t1, tt, u;
	M_PointSet3 b;

	M_PointSetInit3(&b);
	M_PointSetAlloc3(&b, c->n);

	/* XXX: need to do bounds checking on c */

	n = c->n - 1;
	u = t;
	b.p[0].x = c->p[0].x;
	b.p[0].y = c->p[0].y;
	b.p[0].z = c->p[0].z;
	for(k =1; k <=n; k++) {
		b.p[k].x = c->p[k].x *u;
		b.p[k].y = c->p[k].y *u;
		b.p[k].z = c->p[k].z *u;
		u =u*t;
	};

	pt->x = b.p[n].x;
	pt->y = b.p[n].y;
	pt->z = b.p[n].z;
	t1 = 1-t;          tt = t1;
	for(k =n-1; k >=0; k--) {
		pt->x += b.p[k].x *tt;
		pt->y += b.p[k].y *tt;
		pt->z += b.p[k].z *tt;
		tt =tt*t1;
	}

	M_PointSetFree3(&b);
}

