/*
 * Public domain.
 * Operations on vectors in R^2 using standard FPU instructions.
 */

#include <core/core.h>
#include "m.h"

const M_VectorOps2 mVecOps2_FPU = {
	"fpu",
	M_VectorZero2_FPU,
	M_VectorGet2_FPU,
	M_VectorSet2_FPU,
	M_VectorCopy2_FPU,
	M_VectorMirror2_FPU,
	M_VectorMirror2p_FPU,
	M_VectorLen2_FPU,
	M_VectorLen2p_FPU,
	M_VectorDot2_FPU,
	M_VectorDot2p_FPU,
	M_VectorPerpDot2_FPU,
	M_VectorPerpDot2p_FPU,
	M_VectorDistance2_FPU,
	M_VectorDistance2p_FPU,
	M_VectorNorm2_FPU,
	M_VectorNorm2p_FPU,
	M_VectorNorm2v_FPU,
	M_VectorScale2_FPU,
	M_VectorScale2p_FPU,
	M_VectorScale2v_FPU,
	M_VectorAdd2_FPU,
	M_VectorAdd2p_FPU,
	M_VectorAdd2v_FPU,
	M_VectorAdd2n_FPU,
	M_VectorSub2_FPU,
	M_VectorSub2p_FPU,
	M_VectorSub2v_FPU,
	M_VectorSub2n_FPU,
	M_VectorAvg2_FPU,
	M_VectorAvg2p_FPU,
	M_VectorLERP2_FPU,
	M_VectorLERP2p_FPU,
	M_VectorElemPow2_FPU,
	M_VectorVecAngle2_FPU,
	M_VectorRotate2_FPU,
	M_VectorRotate2v_FPU
};

M_Vector2
M_VectorZero2_FPU(void)
{
	M_Vector2 v;

	v.x = 0.0;
	v.y = 0.0;
	return (v);
}

M_Vector2
M_VectorGet2_FPU(M_Real x, M_Real y)
{
	M_Vector2 v;

	v.x = x;
	v.y = y;
	return (v);
}

void
M_VectorSet2_FPU(M_Vector2 *v, M_Real x, M_Real y)
{
	v->x = x;
	v->y = y;
}

void
M_VectorCopy2_FPU(M_Vector2 *vDst, const M_Vector2 *vSrc)
{
	vDst->x = vSrc->x;
	vDst->y = vSrc->y;
}

M_Vector2
M_VectorMirror2_FPU(M_Vector2 a, int x, int y)
{
	M_Vector2 b;

	b.x = x ? -a.x : a.x;
	b.y = y ? -a.y : a.y;
	return (b);
}

M_Vector2
M_VectorMirror2p_FPU(const M_Vector2 *a, int x, int y)
{
	M_Vector2 b;

	b.x = x ? -(a->x) : a->x;
	b.y = y ? -(a->y) : a->y;
	return (b);
}

M_Real
M_VectorLen2_FPU(M_Vector2 v)
{
	return Hypot2(v.x, v.y);
}

M_Real
M_VectorLen2p_FPU(const M_Vector2 *v)
{
	return Hypot2(v->x, v->y);
}

M_Real
M_VectorDot2_FPU(M_Vector2 v1, M_Vector2 v2)
{
	return (v1.x*v2.x + v1.y*v2.y);
}

M_Real
M_VectorDot2p_FPU(const M_Vector2 *v1, const M_Vector2 *v2)
{
	return (v1->x*v2->x + v1->y*v2->y);
}

M_Real
M_VectorPerpDot2_FPU(M_Vector2 v1, M_Vector2 v2)
{
	return (v1.x*v2.y - v1.y*v2.x);
}

M_Real
M_VectorPerpDot2p_FPU(const M_Vector2 *v1, const M_Vector2 *v2)
{
	return (v1->x*v2->y - v1->y*v2->x);
}

M_Real
M_VectorDistance2_FPU(M_Vector2 a, M_Vector2 b)
{
	return M_VectorLen2_FPU( M_VectorSub2_FPU(a,b) );
}

M_Real
M_VectorDistance2p_FPU(const M_Vector2 *a, const M_Vector2 *b)
{
	return M_VectorLen2_FPU( M_VectorAdd2_FPU(*b,
	                         M_VectorMirror2p_FPU(a,1,1)) );
}

M_Vector2
M_VectorNorm2_FPU(M_Vector2 v)
{
	M_Vector2 n;
	M_Real len;

	if ((len = M_VectorLen2p_FPU(&v)) == 0.0) {
		return (v);
	}
	n.x = v.x/len;
	n.y = v.y/len;
	return (n);
}

M_Vector2
M_VectorNorm2p_FPU(const M_Vector2 *v)
{
	M_Vector2 n;
	M_Real len;

	if ((len = M_VectorLen2p_FPU(v)) == 0.0) {
		return (*v);
	}
	n.x = v->x/len;
	n.y = v->y/len;
	return (n);
}

void
M_VectorNorm2v_FPU(M_Vector2 *v)
{
	M_Real len;

	if ((len = M_VectorLen2p_FPU(v)) == 0.0) {
		return;
	}
	v->x /= len;
	v->y /= len;
}

M_Vector2
M_VectorScale2_FPU(M_Vector2 a, M_Real c)
{
	M_Vector2 b;

	b.x = a.x*c;
	b.y = a.y*c;
	return (b);
}

M_Vector2
M_VectorScale2p_FPU(const M_Vector2 *a, M_Real c)
{
	M_Vector2 b;

	b.x = a->x*c;
	b.y = a->y*c;
	return (b);
}

void
M_VectorScale2v_FPU(M_Vector2 *a, M_Real c)
{
	a->x *= c;
	a->y *= c;
}

M_Vector2
M_VectorAdd2_FPU(M_Vector2 a, M_Vector2 b)
{
	M_Vector2 c;

	c.x = a.x + b.x;
	c.y = a.y + b.y;
	return (c);
}

M_Vector2
M_VectorAdd2p_FPU(const M_Vector2 *a, const M_Vector2 *b)
{
	M_Vector2 c;

	c.x = a->x + b->x;
	c.y = a->y + b->y;
	return (c);
}

void
M_VectorAdd2v_FPU(M_Vector2 *r, const M_Vector2 *a)
{
	r->x += a->x;
	r->y += a->y;
}

M_Vector2
M_VectorAdd2n_FPU(int nvecs, ...)
{
	M_Vector2 c, *v;
	int i;
	va_list ap;

	va_start(ap, nvecs);
	v = va_arg(ap, void *);
	c.x = v->x;
	c.y = v->y;
	for (i = 0; i < nvecs; i++) {
		v = va_arg(ap, void *);
		c.x += v->x;
		c.y += v->y;
	}
	va_end(ap);
	return (c);
}

M_Vector2
M_VectorSub2_FPU(M_Vector2 a, M_Vector2 b)
{
	M_Vector2 c;

	c.x = a.x - b.x;
	c.y = a.y - b.y;
	return (c);
}

M_Vector2
M_VectorSub2p_FPU(const M_Vector2 *a, const M_Vector2 *b)
{
	M_Vector2 c;

	c.x = a->x - b->x;
	c.y = a->y - b->y;
	return (c);
}

void
M_VectorSub2v_FPU(M_Vector2 *r, const M_Vector2 *a)
{
	r->x -= a->x;
	r->y -= a->y;
}

M_Vector2
M_VectorSub2n_FPU(int nvecs, ...)
{
	M_Vector2 c, *v;
	int i;
	va_list ap;

	va_start(ap, nvecs);
	v = va_arg(ap, void *);
	c.x = v->x;
	c.y = v->y;
	for (i = 0; i < nvecs; i++) {
		v = va_arg(ap, void *);
		c.x -= v->x;
		c.y -= v->y;
	}
	va_end(ap);
	return (c);
}

M_Vector2
M_VectorAvg2_FPU(M_Vector2 a, M_Vector2 b)
{
	M_Vector2 c;
	
	c.x = (a.x + b.x)/2.0;
	c.y = (a.y + b.y)/2.0;
	return (c);
}

M_Vector2
M_VectorAvg2p_FPU(const M_Vector2 *a, const M_Vector2 *b)
{
	M_Vector2 c;
	
	c.x = (a->x + b->x)/2.0;
	c.y = (a->y + b->y)/2.0;
	return (c);
}

M_Vector2
M_VectorLERP2_FPU(M_Vector2 v1, M_Vector2 v2, M_Real t)
{
	M_Vector2 v;

	v.x = v1.x + (v2.x - v1.x)*t;
	v.y = v1.y + (v2.y - v1.y)*t;
	return (v);
}

M_Vector2
M_VectorLERP2p_FPU(M_Vector2 *v1, M_Vector2 *v2, M_Real t)
{
	M_Vector2 v;

	v.x = v1->x + (v2->x - v1->x)*t;
	v.y = v1->y + (v2->y - v1->y)*t;
	return (v);
}

M_Vector2
M_VectorElemPow2_FPU(M_Vector2 v, M_Real p)
{
	M_Vector2 r;

	r.x = Pow(v.x, p);
	r.y = Pow(v.y, p);
	return (r);
}

M_Real
M_VectorVecAngle2_FPU(M_Vector2 vOrig, M_Vector2 vOther)
{
	M_Vector2 vd;
	
	vd = M_VectorSub2p_FPU(&vOther, &vOrig);
	return (Atan2(vd.y, vd.x));
}

M_Vector2
M_VectorRotate2_FPU(M_Vector2 v, M_Real theta)
{
	M_Vector2 r;
	M_Real s = Sin(theta);
	M_Real c = Cos(theta);
	
	r.x = c*v.x - s*v.y;
	r.y = s*v.x + c*v.y;
	return (r);
}

void
M_VectorRotate2v_FPU(M_Vector2 *v, M_Real theta)
{
	M_Vector2 r;
	M_Real s = Sin(theta);
	M_Real c = Cos(theta);

	r.x = c*v->x - s*v->y;
	r.y = s*v->x + c*v->y;
	M_VectorCopy2_FPU(v, &r);
}
