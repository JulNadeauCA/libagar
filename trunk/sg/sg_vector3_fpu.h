/*
 * Copyright (c) 2007 Hypertriton, Inc. <http://hypertriton.com/>
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
 */

static __inline__  SG_Vector
SG_VectorZero3_FPU(void)
{
	SG_Vector v;

	v.x = 0.0;
	v.y = 0.0;
	v.z = 0.0;
	return (v);
}

static __inline__ SG_Vector
SG_VectorGet3_FPU(SG_Real x, SG_Real y, SG_Real z)
{
	SG_Vector v;

	v.x = x;
	v.y = y;
	v.z = z;
	return (v);
}

static __inline__ void
SG_VectorSet3_FPU(SG_Vector *v, SG_Real x, SG_Real y, SG_Real z)
{
	v->x = x;
	v->y = y;
	v->z = z;
}

static __inline__ void
SG_VectorCopy3_FPU(SG_Vector *vDst, const SG_Vector *vSrc)
{
	vDst->x = vSrc->x;
	vDst->y = vSrc->y;
	vDst->z = vSrc->z;
}

static __inline__ SG_Vector
SG_VectorMirror3_FPU(SG_Vector a, int x, int y, int z)
{
	SG_Vector b;

	b.x = x ? -a.x : a.x;
	b.y = y ? -a.y : a.y;
	b.z = z ? -a.z : a.z;
	return (b);
}

static __inline__ SG_Vector
SG_VectorMirror3p_FPU(const SG_Vector *a, int x, int y, int z)
{
	SG_Vector b;

	b.x = x ? -(a->x) : a->x;
	b.y = y ? -(a->y) : a->y;
	b.z = z ? -(a->z) : a->z;
	return (b);
}

static __inline__ SG_Real
SG_VectorLen3_FPU(SG_Vector v)
{
	return SG_Sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

static __inline__ SG_Real
SG_VectorLen3p_FPU(const SG_Vector *v)
{
	return SG_Sqrt(v->x*v->x + v->y*v->y + v->z*v->z);
}

static __inline__ SG_Real
SG_VectorDot3_FPU(SG_Vector v1, SG_Vector v2)
{
	return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);
}

static __inline__ SG_Real
SG_VectorDot3p_FPU(const SG_Vector *v1, const SG_Vector *v2)
{
	return (v1->x*v2->x + v1->y*v2->y + v1->z*v2->z);
}

static __inline__ SG_Vector
SG_VectorNorm3_FPU(SG_Vector v)
{
	SG_Vector n;
	SG_Real len;

	if ((len = SG_VectorLen3p_FPU(&v)) == 0.0) {
		return (v);
	}
	n.x = v.x/len;
	n.y = v.y/len;
	n.z = v.z/len;
	return (n);
}

static __inline__ SG_Vector
SG_VectorNorm3p_FPU(const SG_Vector *v)
{
	SG_Vector n;
	SG_Real len;

	if ((len = SG_VectorLen3p_FPU(v)) == 0.0) {
		return (*v);
	}
	n.x = v->x/len;
	n.y = v->y/len;
	n.z = v->z/len;
	return (n);
}

static __inline__ void
SG_VectorNorm3v_FPU(SG_Vector *v)
{
	SG_Real len;

	if ((len = SG_VectorLen3p_FPU(v)) == 0.0) {
		return;
	}
	v->x /= len;
	v->y /= len;
	v->z /= len;
}

static __inline__ SG_Vector
SG_VectorCross3_FPU(SG_Vector a, SG_Vector b)
{
	SG_Vector c;

	c.x = a.y*b.z - b.y*a.z;
	c.y = a.z*b.x - b.z*a.x;
	c.z = a.x*b.y - b.x*a.y;
	return (c);
}

static __inline__ SG_Vector
SG_VectorCross3p_FPU(const SG_Vector *a, const SG_Vector *b)
{
	SG_Vector c;

	c.x = a->y*b->z - b->y*a->z;
	c.y = a->z*b->x - b->z*a->x;
	c.z = a->x*b->y - b->x*a->y;
	return (c);
}

static __inline__ SG_Vector
SG_VectorNormCross3_FPU(SG_Vector a, SG_Vector b)
{
	SG_Vector c;

	c.x = a.y*b.z - b.y*a.z;
	c.y = a.z*b.x - b.z*a.x;
	c.z = a.x*b.y - b.x*a.y;
	SG_VectorNorm3v_FPU(&c);
	return (c);
}

static __inline__ SG_Vector
SG_VectorNormCross3p_FPU(const SG_Vector *a, const SG_Vector *b)
{
	SG_Vector c;

	c.x = a->y*b->z - b->y*a->z;
	c.y = a->z*b->x - b->z*a->x;
	c.z = a->x*b->y - b->x*a->y;
	SG_VectorNorm3v_FPU(&c);
	return (c);
}

static __inline__ SG_Vector
SG_VectorScale3_FPU(SG_Vector a, SG_Real c)
{
	SG_Vector b;

	b.x = a.x*c;
	b.y = a.y*c;
	b.z = a.z*c;
	return (b);
}

static __inline__ SG_Vector
SG_VectorScale3p_FPU(const SG_Vector *a, SG_Real c)
{
	SG_Vector b;

	b.x = a->x*c;
	b.y = a->y*c;
	b.z = a->z*c;
	return (b);
}

static __inline__ void
SG_VectorScale3v_FPU(SG_Vector *a, SG_Real c)
{
	a->x *= c;
	a->y *= c;
	a->z *= c;
}

static __inline__ SG_Vector
SG_VectorAdd3_FPU(SG_Vector a, SG_Vector b)
{
	SG_Vector c;

	c.x = a.x + b.x;
	c.y = a.y + b.y;
	c.z = a.z + b.z;
	return (c);
}

static __inline__ SG_Vector
SG_VectorAdd3p_FPU(const SG_Vector *a, const SG_Vector *b)
{
	SG_Vector c;

	c.x = a->x + b->x;
	c.y = a->y + b->y;
	c.z = a->z + b->z;
	return (c);
}

static __inline__ void
SG_VectorAdd3v_FPU(SG_Vector *r, const SG_Vector *a)
{
	r->x += a->x;
	r->y += a->y;
	r->z += a->z;
}

static __inline__ SG_Vector
SG_VectorSub3_FPU(SG_Vector a, SG_Vector b)
{
	SG_Vector c;

	c.x = a.x - b.x;
	c.y = a.y - b.y;
	c.z = a.z - b.z;
	return (c);
}

static __inline__ SG_Vector
SG_VectorSub3p_FPU(const SG_Vector *a, const SG_Vector *b)
{
	SG_Vector c;

	c.x = a->x - b->x;
	c.y = a->y - b->y;
	c.z = a->z - b->z;
	return (c);
}

static __inline__ void
SG_VectorSub3v_FPU(SG_Vector *r, const SG_Vector *a)
{
	r->x -= a->x;
	r->y -= a->y;
	r->z -= a->z;
}

static __inline__ SG_Real
SG_VectorDistance3_FPU(SG_Vector a, SG_Vector b)
{
	return SG_VectorLen3_FPU( SG_VectorSub3_FPU(a,b) );
}

static __inline__ SG_Real
SG_VectorDistance3p_FPU(const SG_Vector *a, const SG_Vector *b)
{
	return SG_VectorLen3_FPU(SG_VectorAdd3_FPU(*b,
	                         SG_VectorMirror3p_FPU(a,1,1,1)));
}

static __inline__ SG_Vector
SG_VectorAvg3_FPU(SG_Vector a, SG_Vector b)
{
	SG_Vector c;
	
	c.x = (a.x + b.x)/2.0;
	c.y = (a.y + b.y)/2.0;
	c.z = (a.z + b.z)/2.0;
	return (c);
}

static __inline__ SG_Vector
SG_VectorAvg3p_FPU(const SG_Vector *a, const SG_Vector *b)
{
	SG_Vector c;
	
	c.x = (a->x + b->x)/2.0;
	c.y = (a->y + b->y)/2.0;
	c.z = (a->z + b->z)/2.0;
	return (c);
}

static __inline__ void
SG_VectorVecAngle3_FPU(SG_Vector vOrig, SG_Vector vOther, SG_Real *theta,
    SG_Real *phi)
{
	SG_Vector vd;

	vd = SG_VectorSub3p_FPU(&vOther, &vOrig);
	if (theta != NULL) {
		*theta = SG_Atan2(vd.y, vd.x);
	}
	if (phi != NULL) {
		*phi = SG_Atan2(SG_Sqrt(vd.x*vd.x + vd.y*vd.y), vd.z);
	}
}

static __inline__ SG_Vector
SG_VectorRotateI3_FPU(SG_Vector a, SG_Real theta)
{
	SG_Vector b;

	b.x = a.x;
	b.y = (a.y * SG_Cos(theta)) + (a.z * -SG_Sin(theta));
	b.z = (a.y * SG_Sin(theta)) + (a.z *  SG_Cos(theta));
	return (b);
}

static __inline__ SG_Vector
SG_VectorRotateJ3_FPU(SG_Vector a, SG_Real theta)
{
	SG_Vector b;

	b.x = (a.x *  SG_Cos(theta)) + (a.z * SG_Sin(theta));
	b.y = a.y;
	b.z = (a.x * -SG_Sin(theta)) + (a.z * SG_Cos(theta));
	return (b);
}

static __inline__ SG_Vector
SG_VectorRotateK3_FPU(SG_Vector a, SG_Real theta)
{
	SG_Vector b;

	b.x = (a.x * SG_Cos(theta)) + (a.y * -SG_Sin(theta));
	b.y = (a.x * SG_Sin(theta)) + (a.y *  SG_Cos(theta));
	b.z = a.z;
	return (b);
}

static __inline__ SG_Vector
SG_VectorLERP3_FPU(SG_Vector v1, SG_Vector v2, SG_Real t)
{
	SG_Vector v;

	v.x = v1.x + (v2.x - v1.x)*t;
	v.y = v1.y + (v2.y - v1.y)*t;
	v.z = v1.z + (v2.z - v1.z)*t;
	return (v);
}

static __inline__ SG_Vector
SG_VectorLERP3p_FPU(SG_Vector *v1, SG_Vector *v2, SG_Real t)
{
	SG_Vector v;

	v.x = v1->x + (v2->x - v1->x)*t;
	v.y = v1->y + (v2->y - v1->y)*t;
	v.z = v1->z + (v2->z - v1->z)*t;
	return (v);
}

static __inline__ SG_Vector
SG_VectorElemPow3_FPU(SG_Vector v, SG_Real pow)
{
	SG_Vector r;

	r.x = SG_Pow(v.x, pow);
	r.y = SG_Pow(v.y, pow);
	r.z = SG_Pow(v.z, pow);
	return (r);
}

__BEGIN_DECLS
extern const SG_VectorOps3 sgVecOps3_FPU;

SG_Vector	SG_VectorAdd3n_FPU(int, ...);
SG_Vector	SG_VectorSub3n_FPU(int, ...);
SG_Vector	SG_VectorRotate3_FPU(SG_Vector, SG_Real, SG_Vector);
void		SG_VectorRotate3v_FPU(SG_Vector *, SG_Real, SG_Vector);
SG_Vector	SG_VectorRotateQuat3_FPU(SG_Vector, SG_Quat);
__END_DECLS
