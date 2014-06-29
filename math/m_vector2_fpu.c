/*
 * Public domain.
 * Operations on vectors in R^2 using scalar instructions.
 */

#include <agar/core/core.h>
#include <agar/math/m.h>

const M_VectorOps2 mVecOps2_FPU = {
	"fpu",
	M_VectorZero2_FPU,
	M_VectorGet2_FPU,
	M_VectorSet2_FPU,
	M_VectorCopy2_FPU,
	M_VectorFlip2_FPU,
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
	M_VectorSum2_FPU,
	M_VectorSub2_FPU,
	M_VectorSub2p_FPU,
	M_VectorSub2v_FPU,
	M_VectorAvg2_FPU,
	M_VectorAvg2p_FPU,
	M_VectorLERP2_FPU,
	M_VectorLERP2p_FPU,
	M_VectorElemPow2_FPU,
	M_VectorVecAngle2_FPU
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
M_VectorFlip2_FPU(M_Vector2 a)
{
	M_Vector2 b;

	b.x = -a.x;
	b.y = -a.y;
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
	                         M_VectorFlip2_FPU(*a)) );
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
M_VectorSum2_FPU(const M_Vector2 *va, Uint count)
{
	M_Vector2 v;
	Uint i;
	
	v.x = 0.0;
	v.y = 0.0;
	for (i = 0; i < count; i++) {
		v.x += va[i].x;
		v.y += va[i].y;
	}
	return (v);
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
