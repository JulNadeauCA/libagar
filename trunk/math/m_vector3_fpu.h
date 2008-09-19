/*
 * Copyright (c) 2007-2008 Hypertriton, Inc. <http://hypertriton.com/>
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

__BEGIN_DECLS
static __inline__ M_Vector3
M_VectorZero3_FPU(void)
{
	M_Vector3 v;
	
	v.x = 0.0;
	v.y = 0.0;
	v.z = 0.0;
	return (v);
}

static __inline__ M_Vector3
M_VectorGet3_FPU(M_Real x, M_Real y, M_Real z)
{
	M_Vector3 v;

	v.x = x;
	v.y = y;
	v.z = z;
	return (v);
}

static __inline__ void
M_VectorSet3_FPU(M_Vector3 *v, M_Real x, M_Real y, M_Real z)
{
	v->x = x;
	v->y = y;
	v->z = z;
}

static __inline__ void
M_VectorCopy3_FPU(M_Vector3 *vDst, const M_Vector3 *vSrc)
{
	vDst->x = vSrc->x;
	vDst->y = vSrc->y;
	vDst->z = vSrc->z;
}

static __inline__ M_Vector3
M_VectorMirror3_FPU(M_Vector3 a, int x, int y, int z)
{
	M_Vector3 b;

	b.x = x ? -a.x : a.x;
	b.y = y ? -a.y : a.y;
	b.z = z ? -a.z : a.z;
	return (b);
}

static __inline__ M_Vector3
M_VectorMirror3p_FPU(const M_Vector3 *a, int x, int y, int z)
{
	M_Vector3 b;
	
	b.x = x ? -(a->x) : a->x;
	b.y = y ? -(a->y) : a->y;
	b.z = z ? -(a->z) : a->z;
	return (b);
}

static __inline__ M_Real
M_VectorLen3_FPU(M_Vector3 v)
{
	return M_Sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}

static __inline__ M_Real
M_VectorLen3p_FPU(const M_Vector3 *v)
{
	return M_Sqrt(v->x*v->x + v->y*v->y + v->z*v->z);
}

static __inline__ M_Real
M_VectorDot3_FPU(M_Vector3 v1, M_Vector3 v2)
{
	return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);
}

static __inline__ M_Real
M_VectorDot3p_FPU(const M_Vector3 *v1, const M_Vector3 *v2)
{
	return (v1->x*v2->x + v1->y*v2->y + v1->z*v2->z);
}

static __inline__ M_Vector3
M_VectorNorm3_FPU(M_Vector3 v)
{
	M_Vector3 n;
	M_Real len;

	if ((len = M_VectorLen3p_FPU(&v)) == 0.0) {
		return (v);
	}
	n.x = v.x/len;
	n.y = v.y/len;
	n.z = v.z/len;
	return (n);
}

static __inline__ M_Vector3
M_VectorNorm3p_FPU(const M_Vector3 *v)
{
	M_Vector3 n;
	M_Real len;

	if ((len = M_VectorLen3p_FPU(v)) == 0.0) {
		return (*v);
	}
	n.x = v->x/len;
	n.y = v->y/len;
	n.z = v->z/len;
	return (n);
}

static __inline__ void
M_VectorNorm3v_FPU(M_Vector3 *v)
{
	M_Real len;

	if ((len = M_VectorLen3p_FPU(v)) == 0.0) {
		return;
	}
	v->x /= len;
	v->y /= len;
	v->z /= len;
}

static __inline__ M_Vector3
M_VectorCross3_FPU(M_Vector3 a, M_Vector3 b)
{
	M_Vector3 c;

	c.x = a.y*b.z - b.y*a.z;
	c.y = a.z*b.x - b.z*a.x;
	c.z = a.x*b.y - b.x*a.y;
	return (c);
}

static __inline__ M_Vector3
M_VectorCross3p_FPU(const M_Vector3 *a, const M_Vector3 *b)
{
	M_Vector3 c;

	c.x = a->y*b->z - b->y*a->z;
	c.y = a->z*b->x - b->z*a->x;
	c.z = a->x*b->y - b->x*a->y;
	return (c);
}

static __inline__ M_Vector3
M_VectorNormCross3_FPU(M_Vector3 a, M_Vector3 b)
{
	M_Vector3 c;

	c.x = a.y*b.z - b.y*a.z;
	c.y = a.z*b.x - b.z*a.x;
	c.z = a.x*b.y - b.x*a.y;
	M_VectorNorm3v_FPU(&c);
	return (c);
}

static __inline__ M_Vector3
M_VectorNormCross3p_FPU(const M_Vector3 *a, const M_Vector3 *b)
{
	M_Vector3 c;

	c.x = a->y*b->z - b->y*a->z;
	c.y = a->z*b->x - b->z*a->x;
	c.z = a->x*b->y - b->x*a->y;
	M_VectorNorm3v_FPU(&c);
	return (c);
}

static __inline__ M_Vector3
M_VectorScale3_FPU(M_Vector3 a, M_Real c)
{
	M_Vector3 b;

	b.x = a.x*c;
	b.y = a.y*c;
	b.z = a.z*c;
	return (b);
}

static __inline__ M_Vector3
M_VectorScale3p_FPU(const M_Vector3 *a, M_Real c)
{
	M_Vector3 b;

	b.x = a->x*c;
	b.y = a->y*c;
	b.z = a->z*c;
	return (b);
}

static __inline__ void
M_VectorScale3v_FPU(M_Vector3 *a, M_Real c)
{
	a->x *= c;
	a->y *= c;
	a->z *= c;
}

static __inline__ M_Vector3
M_VectorAdd3_FPU(M_Vector3 a, M_Vector3 b)
{
	M_Vector3 c;

	c.x = a.x + b.x;
	c.y = a.y + b.y;
	c.z = a.z + b.z;
	return (c);
}

static __inline__ M_Vector3
M_VectorAdd3p_FPU(const M_Vector3 *a, const M_Vector3 *b)
{
	M_Vector3 c;

	c.x = a->x + b->x;
	c.y = a->y + b->y;
	c.z = a->z + b->z;
	return (c);
}

static __inline__ void
M_VectorAdd3v_FPU(M_Vector3 *r, const M_Vector3 *a)
{
	r->x += a->x;
	r->y += a->y;
	r->z += a->z;
}

static __inline__ M_Vector3
M_VectorSub3_FPU(M_Vector3 a, M_Vector3 b)
{
	M_Vector3 c;

	c.x = a.x - b.x;
	c.y = a.y - b.y;
	c.z = a.z - b.z;
	return (c);
}

static __inline__ M_Vector3
M_VectorSub3p_FPU(const M_Vector3 *a, const M_Vector3 *b)
{
	M_Vector3 c;

	c.x = a->x - b->x;
	c.y = a->y - b->y;
	c.z = a->z - b->z;
	return (c);
}

static __inline__ void
M_VectorSub3v_FPU(M_Vector3 *r, const M_Vector3 *a)
{
	r->x -= a->x;
	r->y -= a->y;
	r->z -= a->z;
}

static __inline__ M_Real
M_VectorDistance3_FPU(M_Vector3 a, M_Vector3 b)
{
	return M_VectorLen3_FPU( M_VectorSub3_FPU(a,b) );
}

static __inline__ M_Real
M_VectorDistance3p_FPU(const M_Vector3 *a, const M_Vector3 *b)
{
	return M_VectorLen3_FPU(M_VectorAdd3_FPU(*b,
	                        M_VectorMirror3p_FPU(a,1,1,1)));
}

static __inline__ M_Vector3
M_VectorAvg3_FPU(M_Vector3 a, M_Vector3 b)
{
	M_Vector3 c;
	
	c.x = (a.x + b.x)/2.0;
	c.y = (a.y + b.y)/2.0;
	c.z = (a.z + b.z)/2.0;
	return (c);
}

static __inline__ M_Vector3
M_VectorAvg3p_FPU(const M_Vector3 *a, const M_Vector3 *b)
{
	M_Vector3 c;
	
	c.x = (a->x + b->x)/2.0;
	c.y = (a->y + b->y)/2.0;
	c.z = (a->z + b->z)/2.0;
	return (c);
}

static __inline__ void
M_VectorVecAngle3_FPU(M_Vector3 vOrig, M_Vector3 vOther, M_Real *theta,
    M_Real *phi)
{
	M_Vector3 vd;

	vd = M_VectorSub3p_FPU(&vOther, &vOrig);
	if (theta != NULL) {
		*theta = M_Atan2(vd.y, vd.x);
	}
	if (phi != NULL) {
		*phi = M_Atan2(M_Sqrt(vd.x*vd.x + vd.y*vd.y), vd.z);
	}
}

static __inline__ M_Vector3
M_VectorRotateI3_FPU(M_Vector3 a, M_Real theta)
{
	M_Vector3 b;

	b.x = a.x;
	b.y = (a.y * M_Cos(theta)) + (a.z * -M_Sin(theta));
	b.z = (a.y * M_Sin(theta)) + (a.z *  M_Cos(theta));
	return (b);
}

static __inline__ M_Vector3
M_VectorRotateJ3_FPU(M_Vector3 a, M_Real theta)
{
	M_Vector3 b;

	b.x = (a.x *  M_Cos(theta)) + (a.z * M_Sin(theta));
	b.y = a.y;
	b.z = (a.x * -M_Sin(theta)) + (a.z * M_Cos(theta));
	return (b);
}

static __inline__ M_Vector3
M_VectorRotateK3_FPU(M_Vector3 a, M_Real theta)
{
	M_Vector3 b;

	b.x = (a.x * M_Cos(theta)) + (a.y * -M_Sin(theta));
	b.y = (a.x * M_Sin(theta)) + (a.y *  M_Cos(theta));
	b.z = a.z;
	return (b);
}

static __inline__ M_Vector3
M_VectorLERP3_FPU(M_Vector3 v1, M_Vector3 v2, M_Real t)
{
	M_Vector3 v;

	v.x = v1.x + (v2.x - v1.x)*t;
	v.y = v1.y + (v2.y - v1.y)*t;
	v.z = v1.z + (v2.z - v1.z)*t;
	return (v);
}

static __inline__ M_Vector3
M_VectorLERP3p_FPU(M_Vector3 *v1, M_Vector3 *v2, M_Real t)
{
	M_Vector3 v;

	v.x = v1->x + (v2->x - v1->x)*t;
	v.y = v1->y + (v2->y - v1->y)*t;
	v.z = v1->z + (v2->z - v1->z)*t;
	return (v);
}

static __inline__ M_Vector3
M_VectorElemPow3_FPU(M_Vector3 v, M_Real x)
{
	M_Vector3 r;

	r.x = M_Pow(v.x, x);
	r.y = M_Pow(v.y, x);
	r.z = M_Pow(v.z, x);
	return (r);
}
__END_DECLS

__BEGIN_DECLS
extern const M_VectorOps3 mVecOps3_FPU;

M_Vector3	M_VectorAdd3n_FPU(int, ...);
M_Vector3	M_VectorSub3n_FPU(int, ...);
M_Vector3	M_VectorRotate3_FPU(M_Vector3, M_Real, M_Vector3);
void		M_VectorRotate3v_FPU(M_Vector3 *, M_Real, M_Vector3);
M_Vector3	M_VectorRotateQuat3_FPU(M_Vector3, M_Quaternion);
__END_DECLS
