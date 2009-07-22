/*
 * Public domain.
 * Operations on vectors in R^4 using standard FPU instructions.
 */

#include <core/core.h>
#include "m.h"

const M_VectorOps4 mVecOps4_FPU = {
	"fpu",
	M_VectorZero4_FPU,
	M_VectorGet4_FPU,
	M_VectorSet4_FPU,
	M_VectorCopy4_FPU,
	M_VectorMirror4_FPU,
	M_VectorMirror4p_FPU,
	M_VectorLen4_FPU,
	M_VectorLen4p_FPU,
	M_VectorDot4_FPU,
	M_VectorDot4p_FPU,
	M_VectorDistance4_FPU,
	M_VectorDistance4p_FPU,
	M_VectorNorm4_FPU,
	M_VectorNorm4p_FPU,
	M_VectorNorm4v_FPU,
	M_VectorScale4_FPU,
	M_VectorScale4p_FPU,
	M_VectorScale4v_FPU,
	M_VectorAdd4_FPU,
	M_VectorAdd4p_FPU,
	M_VectorAdd4v_FPU,
	M_VectorAdd4n_FPU,
	M_VectorSub4_FPU,
	M_VectorSub4p_FPU,
	M_VectorSub4v_FPU,
	M_VectorSub4n_FPU,
	M_VectorAvg4_FPU,
	M_VectorAvg4p_FPU,
	M_VectorLERP4_FPU,
	M_VectorLERP4p_FPU,
	M_VectorElemPow4_FPU,
	M_VectorVecAngle4_FPU,
	M_VectorRotate4_FPU,
	M_VectorRotate4v_FPU
};

M_Vector4
M_VectorZero4_FPU(void)
{
	M_Vector4 v;

	v.x = 0.0;
	v.y = 0.0;
	v.z = 0.0;
	v.w = 0.0;
	return (v);
}

M_Vector4
M_VectorGet4_FPU(M_Real x, M_Real y, M_Real z, M_Real w)
{
	M_Vector4 v;

	v.x = x;
	v.y = y;
	v.z = z;
	v.w = w;
	return (v);
}

void
M_VectorSet4_FPU(M_Vector4 *v, M_Real x, M_Real y, M_Real z, M_Real w)
{
	v->x = x;
	v->y = y;
	v->z = z;
	v->w = w;
}

void
M_VectorCopy4_FPU(M_Vector4 *vDst, const M_Vector4 *vSrc)
{
	vDst->x = vSrc->x;
	vDst->y = vSrc->y;
	vDst->z = vSrc->z;
	vDst->w = vSrc->w;
}

M_Vector4
M_VectorMirror4_FPU(M_Vector4 a, int x, int y, int z, int w)
{
	M_Vector4 b;

	b.x = x ? -a.x : a.x;
	b.y = y ? -a.y : a.y;
	b.z = z ? -a.z : a.z;
	b.w = w ? -a.w : a.w;
	return (b);
}

M_Vector4
M_VectorMirror4p_FPU(const M_Vector4 *a, int x, int y, int z, int w)
{
	M_Vector4 b;

	b.x = x ? -(a->x) : a->x;
	b.y = y ? -(a->y) : a->y;
	b.z = z ? -(a->z) : a->z;
	b.w = w ? -(a->w) : a->w;
	return (b);
}

M_Real
M_VectorLen4_FPU(M_Vector4 v)
{
	return Sqrt(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w);
}

M_Real
M_VectorLen4p_FPU(const M_Vector4 *v)
{
	return Sqrt(v->x*v->x + v->y*v->y + v->z*v->z + v->w*v->w);
}

M_Real
M_VectorDot4_FPU(M_Vector4 v1, M_Vector4 v2)
{
	return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z + v1.w*v2.w);
}

M_Real
M_VectorDot4p_FPU(const M_Vector4 *v1, const M_Vector4 *v2)
{
	return (v1->x*v2->x + v1->y*v2->y + v1->z*v2->z + v1->w*v2->w);
}

M_Real
M_VectorDistance4_FPU(M_Vector4 a, M_Vector4 b)
{
	return M_VectorLen4_FPU( M_VectorSub4_FPU(a,b) );
}

M_Real
M_VectorDistance4p_FPU(const M_Vector4 *a, const M_Vector4 *b)
{
	return M_VectorLen4_FPU( M_VectorAdd4_FPU(*b,
	                         M_VectorMirror4p_FPU(a,1,1,1,1)) );
}

M_Vector4
M_VectorNorm4_FPU(M_Vector4 v)
{
	M_Vector4 n;
	M_Real len;

	if ((len = M_VectorLen4p_FPU(&v)) == 0.0) {
		return (v);
	}
	n.x = v.x/len;
	n.y = v.y/len;
	n.z = v.z/len;
	n.w = v.w/len;
	return (n);
}

M_Vector4
M_VectorNorm4p_FPU(const M_Vector4 *v)
{
	M_Vector4 n;
	M_Real len;

	if ((len = M_VectorLen4p_FPU(v)) == 0.0) {
		return (*v);
	}
	n.x = v->x/len;
	n.y = v->y/len;
	n.z = v->z/len;
	n.w = v->w/len;
	return (n);
}

void
M_VectorNorm4v_FPU(M_Vector4 *v)
{
	M_Real len;

	if ((len = M_VectorLen4p_FPU(v)) == 0.0) {
		return;
	}
	v->x /= len;
	v->y /= len;
	v->z /= len;
	v->w /= len;
}

M_Vector4
M_VectorScale4_FPU(M_Vector4 a, M_Real c)
{
	M_Vector4 b;

	b.x = a.x*c;
	b.y = a.y*c;
	b.z = a.z*c;
	b.w = a.w*c;
	return (b);
}

M_Vector4
M_VectorScale4p_FPU(const M_Vector4 *a, M_Real c)
{
	M_Vector4 b;

	b.x = a->x*c;
	b.y = a->y*c;
	b.z = a->z*c;
	b.w = a->w*c;
	return (b);
}

void
M_VectorScale4v_FPU(M_Vector4 *a, M_Real c)
{
	a->x *= c;
	a->y *= c;
	a->z *= c;
	a->w *= c;
}

M_Vector4
M_VectorAdd4_FPU(M_Vector4 a, M_Vector4 b)
{
	M_Vector4 c;

	c.x = a.x + b.x;
	c.y = a.y + b.y;
	c.z = a.z + b.z;
	c.w = a.w + b.w;
	return (c);
}

M_Vector4
M_VectorAdd4p_FPU(const M_Vector4 *a, const M_Vector4 *b)
{
	M_Vector4 c;

	c.x = a->x + b->x;
	c.y = a->y + b->y;
	c.z = a->z + b->z;
	c.w = a->w + b->w;
	return (c);
}

void
M_VectorAdd4v_FPU(M_Vector4 *r, const M_Vector4 *a)
{
	r->x += a->x;
	r->y += a->y;
	r->z += a->z;
	r->w += a->w;
}

M_Vector4
M_VectorAdd4n_FPU(int nvecs, ...)
{
	M_Vector4 c, *v;
	int i;
	va_list ap;

	va_start(ap, nvecs);
	v = va_arg(ap, void *);
	c.x = v->x;
	c.y = v->y;
	c.z = v->z;
	c.w = v->w;
	for (i = 0; i < nvecs; i++) {
		v = va_arg(ap, void *);
		c.x += v->x;
		c.y += v->y;
		c.z += v->z;
		c.w += v->w;
	}
	va_end(ap);
	return (c);
}

M_Vector4
M_VectorSub4_FPU(M_Vector4 a, M_Vector4 b)
{
	M_Vector4 c;

	c.x = a.x - b.x;
	c.y = a.y - b.y;
	c.z = a.z - b.z;
	c.w = a.w - b.w;
	return (c);
}

M_Vector4
M_VectorSub4p_FPU(const M_Vector4 *a, const M_Vector4 *b)
{
	M_Vector4 c;

	c.x = a->x - b->x;
	c.y = a->y - b->y;
	c.z = a->z - b->z;
	c.w = a->w - b->w;
	return (c);
}

void
M_VectorSub4v_FPU(M_Vector4 *r, const M_Vector4 *a)
{
	r->x -= a->x;
	r->y -= a->y;
	r->z -= a->z;
	r->w -= a->w;
}

M_Vector4
M_VectorSub4n_FPU(int nvecs, ...)
{
	M_Vector4 c, *v;
	int i;
	va_list ap;

	va_start(ap, nvecs);
	v = va_arg(ap, void *);
	c.x = v->x;
	c.y = v->y;
	c.z = v->z;
	c.w = v->w;
	for (i = 0; i < nvecs; i++) {
		v = va_arg(ap, void *);
		c.x -= v->x;
		c.y -= v->y;
		c.z -= v->z;
		c.w -= v->w;
	}
	va_end(ap);
	return (c);
}

M_Vector4
M_VectorAvg4_FPU(M_Vector4 a, M_Vector4 b)
{
	M_Vector4 c;
	
	c.x = (a.x + b.x)/2.0;
	c.y = (a.y + b.y)/2.0;
	c.z = (a.z + b.z)/2.0;
	c.w = (a.w + b.w)/2.0;
	return (c);
}

M_Vector4
M_VectorAvg4p_FPU(const M_Vector4 *a, const M_Vector4 *b)
{
	M_Vector4 c;
	
	c.x = (a->x + b->x)/2.0;
	c.y = (a->y + b->y)/2.0;
	c.z = (a->z + b->z)/2.0;
	c.w = (a->w + b->w)/2.0;
	return (c);
}

void
M_VectorVecAngle4_FPU(M_Vector4 vOrig, M_Vector4 vOther, M_Real *phi1,
    M_Real *phi2, M_Real *phi3)
{
	M_Vector4 vd;

	vd = M_VectorSub4p_FPU(&vOther, &vOrig);
	if (phi1 != NULL)
		*phi1 = Atan2(Sqrt(vd.w*vd.w + vd.z*vd.z + vd.y*vd.y),	vd.x);
	if (phi2 != NULL)
		*phi2 = Atan2(Sqrt(vd.w*vd.w + vd.z*vd.z),		vd.y);
	if (phi3 != NULL)
		*phi3 = Atan2(vd.w,					vd.z);
}

M_Vector4
M_VectorRotate4_FPU(M_Vector4 v, M_Real theta, M_Vector4 n)
{
	M_Vector4 r = v;

	M_VectorRotate4v_FPU(&r, theta, n);
	return (r);
}

void
M_VectorRotate4v_FPU(M_Vector4 *v, M_Real theta, M_Vector4 n)
{
	M_Real s = Sin(theta);
	M_Real c = Cos(theta);
	M_Real t = 1.0 - c;
	M_Matrix44 R;

	R.m[0][0] = 1.0 - (n.x*n.x)*t;
	R.m[0][1] = -t*n.x*n.y;
	R.m[0][2] = -t*n.x*n.z;
	R.m[0][3] =  s*n.x;
	R.m[1][0] = -t*n.x*n.y;
	R.m[1][1] = 1.0 - (n.y*n.y)*t;
	R.m[1][2] = -t*n.y*n.z;
	R.m[1][3] =  s*n.y;
	R.m[2][0] = -t*n.x*n.z;
	R.m[2][1] = -t*n.y*n.z;
	R.m[2][2] = 1.0 - (n.z*n.z)*t;
	R.m[2][3] =  s*n.z;
	R.m[3][0] = -s*n.x;
	R.m[3][1] = -s*n.y;
	R.m[3][2] = -s*n.z;
	R.m[3][3] = c;
	M_MatMult44Vector4v(v, &R);
}

M_Vector4
M_VectorLERP4_FPU(M_Vector4 v1, M_Vector4 v2, M_Real t)
{
	M_Vector4 v;

	v.x = v1.x + (v2.x - v1.x)*t;
	v.y = v1.y + (v2.y - v1.y)*t;
	v.z = v1.z + (v2.z - v1.z)*t;
	v.w = v1.w + (v2.w - v1.w)*t;
	return (v);
}

M_Vector4
M_VectorLERP4p_FPU(M_Vector4 *v1, M_Vector4 *v2, M_Real t)
{
	M_Vector4 v;

	v.x = v1->x + (v2->x - v1->x)*t;
	v.y = v1->y + (v2->y - v1->y)*t;
	v.z = v1->z + (v2->z - v1->z)*t;
	v.w = v1->w + (v2->w - v1->w)*t;
	return (v);
}

M_Vector4
M_VectorElemPow4_FPU(M_Vector4 v, M_Real p)
{
	M_Vector4 r;

	r.x = Pow(v.x, p);
	r.y = Pow(v.y, p);
	r.z = Pow(v.z, p);
	r.w = Pow(v.w, p);
	return (r);
}
