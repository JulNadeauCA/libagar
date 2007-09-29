/*
 * Copyright (c) 2005-2007 Hypertriton, Inc. <http://hypertriton.com/>
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

/*
 * Quaternion operations.
 */

#include <config/have_opengl.h>
#ifdef HAVE_OPENGL

#include <core/core.h>

#include "sg.h"

#define SG_SLERP_TO_LERP_THRESH 0.001

SG_Quat
SG_QuatMultIdentity(void)
{
	SG_Quat q;

	q.w = 1.0;
	q.x = 0.0;
	q.y = 0.0;
	q.z = 0.0;
	return (q);
}

SG_Quat
SG_QuatAddIdentity(void)
{
	SG_Quat q;

	q.w = 0.0;
	q.x = 0.0;
	q.y = 0.0;
	q.z = 0.0;
	return (q);
}

void
SG_QuatToMatrix(SG_Matrix4 *A, const SG_Quat *q)
{
	SG_Real x = q->x;
	SG_Real y = q->y;
	SG_Real z = q->z;
	SG_Real w = q->w;

	A->m[0][0] = 1 - 2*y*y - 2*z*z;
	A->m[0][1] =     2*x*y - 2*w*z;
	A->m[0][2] =     2*x*z + 2*w*y;
	A->m[0][3] = 0.0;

	A->m[1][0] =     2*x*y + 2*w*z;
	A->m[1][1] = 1 - 2*x*x - 2*z*z;
	A->m[1][2] =     2*y*z - 2*w*x;
	A->m[1][3] = 0.0;

	A->m[2][0] =     2*x*z - 2*w*y;
	A->m[2][1] =     2*y*z + 2*w*x;
	A->m[2][2] = 1 - 2*x*x - 2*y*y;
	A->m[2][3] = 0.0;
	
	A->m[3][0] = 0.0;
	A->m[3][1] = 0.0;
	A->m[3][2] = 0.0;
	A->m[3][3] = 1.0;
}

void
SG_QuatpToAxisAngle(const SG_Quat *q, SG_Vector *v, SG_Real *theta)
{
	SG_Real s;
	
	s = SG_Sqrt(q->x*q->x + q->y*q->y + q->z*q->z);
	if (s != 0.0) {
		*theta = 2.0*SG_Acos(q->w);
		v->x = q->x/s;
		v->y = q->y/s;
		v->z = q->z/s;
	} else {
		*theta = 0.0;
		v->x = 1.0;
		v->y = 0.0;
		v->z = 0.0;
	}
}

void
SG_QuatpToAxisAngle3(const SG_Quat *q, SG_Real *theta, SG_Real *x, SG_Real *y,
    SG_Real *z)
{
	SG_Vector axis;
	
	SG_QuatpToAxisAngle(q, &axis, theta);
	*x = axis.x;
	*y = axis.y;
	*z = axis.z;
}

SG_Quat
SG_QuatFromAxisAngle(SG_Vector axis, SG_Real theta)
{
	SG_Quat q;
	SG_Real s;

	s = SG_Sin(theta/2.0);
	q.w = SG_Cos(theta/2.0);
	q.x = axis.x*s;
	q.y = axis.y*s;
	q.z = axis.z*s;
	SG_QuatNormv(&q);
	return (q);
}

SG_Quat
SG_QuatFromAxisAngle3(SG_Real theta, SG_Real x, SG_Real y, SG_Real z)
{
	SG_Quat q;
	SG_Real s;

	s = SG_Sin(theta/2.0);
	q.w = SG_Cos(theta/2.0);
	q.x = x*s;
	q.y = y*s;
	q.z = z*s;
	SG_QuatNormv(&q);
	return (q);
}

void
SG_QuatpFromAxisAngle(SG_Quat *q, SG_Vector axis, SG_Real theta)
{
	SG_Real s;

	s = SG_Sin(theta/2.0);
	q->w = SG_Cos(theta/2.0);
	q->x = axis.x*s;
	q->y = axis.y*s;
	q->z = axis.z*s;
	SG_QuatNormv(q);
}

void
SG_QuatpFromAxisAngle3(SG_Quat *q, SG_Real theta, SG_Real x, SG_Real y,
    SG_Real z)
{
	SG_Real s, len;

	s = SG_Sin(theta/2.0);
	q->w = SG_Cos(theta/2.0);
	q->x = x*s;
	q->y = y*s;
	q->z = z*s;
	len = SG_Sqrt(q->x*q->x + q->y*q->y + q->z*q->z + q->w*q->w);
	q->w /= len;
	q->x /= len;
	q->y /= len;
	q->z /= len;
}

void
SG_QuatFromEulv(SG_Quat *Qr, SG_Real a, SG_Real b, SG_Real c)
{
	SG_Quat Qx, Qy, Qv;

	Qx.w = SG_Cos(a/2.0);
	Qx.x = SG_Sin(a/2.0);
	Qx.y = 0.0;
	Qx.z = 0.0;

	Qy.w = SG_Cos(b/2.0);
	Qy.x = 0.0;
	Qy.y = SG_Sin(b/2.0);
	Qy.z = 0.0;
	
	Qy.w = SG_Cos(c/2.0);
	Qy.x = 0.0;
	Qy.y = 0.0;
	Qy.z = SG_Sin(b/2.0);

	*Qr = SG_QuatMult3(Qx, Qy, Qx);
}

SG_Quat
SG_QuatFromEul(SG_Real a, SG_Real b, SG_Real c)
{
	SG_Quat Qr;

	SG_QuatFromEulv(&Qr, a, b, c);
	return (Qr);
}

SG_Quat
SG_QuatConj(SG_Quat q)
{
	return (SG_QuatConjp(&q));
}

SG_Quat
SG_QuatConjp(const SG_Quat *q)
{
	SG_Quat nq;

	nq.x = -(q->x);
	nq.y = -(q->y);
	nq.z = -(q->z);
	nq.w = q->w;
	return (nq);
}

void
SG_QuatConjv(SG_Quat *q)
{
	q->x = -(q->x);
	q->y = -(q->y);
	q->z = -(q->z);
}

SG_Quat
SG_QuatScale(SG_Quat q, SG_Real c)
{
	return (SG_QuatScalep(&q, c));
}

SG_Quat
SG_QuatScalep(const SG_Quat *q, SG_Real c)
{
	SG_Quat nq;

	nq.x = q->x*c;
	nq.y = q->y*c;
	nq.z = q->z*c;
	nq.w = q->w*c;
	return (nq);
}

void
SG_QuatScalev(SG_Quat *q, SG_Real c)
{
	q->x *= c;
	q->y *= c;
	q->z *= c;
	q->w *= c;
}

SG_Quat
SG_QuatMult(SG_Quat q1, SG_Quat q2)
{
	return (SG_QuatMultp(&q1, &q2));
}

SG_Quat
SG_QuatMultp(const SG_Quat *A, const SG_Quat *B)
{
	SG_Quat C;

	C.w = A->w*B->w - A->x*B->x - A->y*B->y - A->z*B->z;
	C.x = A->w*B->x + A->x*B->w + A->y*B->z - A->z*B->y;
	C.y = A->w*B->y - A->x*B->z + A->y*B->w + A->z*B->x;
	C.z = A->w*B->z + A->x*B->y - A->y*B->x + A->z*B->w;
	return (C);
}

void
SG_QuatMultv(SG_Quat *C, const SG_Quat *A, const SG_Quat *B)
{
	C->w = A->w*B->w - A->x*B->x - A->y*B->y - A->z*B->z;
	C->x = A->w*B->x + A->x*B->w + A->y*B->z - A->z*B->y;
	C->y = A->w*B->y - A->x*B->z + A->y*B->w + A->z*B->x;
	C->z = A->w*B->z + A->x*B->y - A->y*B->x + A->z*B->w;
}

/* Alternative to SG_QuatMult(). */
SG_Quat
SG_QuatConcat(const SG_Quat *A, const SG_Quat *B)
{
	SG_Vector v1, v2, cross;
	SG_Quat R;
	SG_Real angle;

	v1.x = A->x;
	v1.y = A->y;
	v1.z = A->z;
	v2.x = B->x;
	v2.y = B->y;
	v2.z = B->z;
	angle = A->w*B->w - VecDotp(&v1, &v2);
	cross = VecCrossp(&v1, &v2);
	v1.x *= B->w;
	v1.y *= B->w;
	v1.z *= B->w;
	v2.x *= A->w;
	v2.y *= A->w;
	v2.z *= A->w;

	R.w = angle;
	R.x = v1.x + v2.x + cross.x;
	R.y = v1.y + v2.y + cross.y;
	R.z = v1.z + v2.z + cross.z;
	return (R);
}

SG_Quat
SG_QuatNorm(SG_Quat q)
{
	return (SG_QuatNormp(&q));
}

SG_Quat
SG_QuatNormp(const SG_Quat *q)
{
	SG_Quat nq;
	SG_Real s;

	s = SG_Sqrt(q->x*q->x + q->y*q->y + q->z*q->z + q->w*q->w);
	if (s == 0.0) {
		nq = *q;
		return (nq);
	}
	nq.x = q->x/s;
	nq.y = q->y/s;
	nq.z = q->z/s;
	nq.w = q->w/s;
	return (nq);
}

void
SG_QuatNormv(SG_Quat *q)
{
	SG_Real s;

	s = SG_Sqrt(q->x*q->x + q->y*q->y + q->z*q->z + q->w*q->w);
	if (s == 0.0) {
		return;
	}
	q->x /= s;
	q->y /= s;
	q->z /= s;
	q->w /= s;
}

SG_Quat
SG_QuatInverse(SG_Quat Q)
{
	SG_Quat Qc;

	Qc = SG_QuatConjp(&Q);
	SG_QuatNormv(&Qc);
	return (Qc);
}

SG_Quat
SG_QuatInversep(const SG_Quat *Q)
{
	SG_Quat Qc;

	Qc = SG_QuatConjp(Q);
	SG_QuatNormv(&Qc);
	return (Qc);
}

void
SG_QuatInversev(SG_Quat *Q)
{
	SG_QuatConjv(Q);
	SG_QuatNormv(Q);
}

SG_Quat
SG_QuatSLERP(SG_Quat q1, SG_Quat q2, SG_Real alpha)
{
	return (SG_QuatSLERPp(&q1, &q2, alpha));
}

SG_Quat
SG_QuatSLERPp(const SG_Quat *q1, const SG_Quat *q2, SG_Real alpha)
{
	SG_Real o, co, so, scale0, scale1;
	SG_Real qi[4];
	SG_Quat qr;

	co = q1->x*q2->x + q1->y*q2->y + q1->z*q2->z + q1->w*q2->w;

	if (co < 0.0) {
		co = -co;
		qi[0] = -q2->x;
		qi[1] = -q2->y;
		qi[2] = -q2->z;
		qi[3] = -q2->w;
	} else {
		qi[0] = q2->x;
		qi[1] = q2->y;
		qi[2] = q2->z;
		qi[3] = q2->w;
	}

	if ((1 - co) < SG_SLERP_TO_LERP_THRESH) {
		scale0 = 1 - alpha;
		scale1 = alpha;
	} else {
		o = SG_Acos(co);
		so = SG_Sin(o);
		scale0 = SG_Sin((1-alpha)*o)/so;
		scale1 = SG_Sin(alpha*o)/so;
	}

	qr.w = scale0*q1->w + scale1*qi[3];
	qr.x = scale0*q1->x + scale1*qi[0];
	qr.y = scale0*q1->y + scale1*qi[1];
	qr.z = scale0*q1->z + scale1*qi[2];
	return (qr);
}

SG_Quat
SG_ReadQuat(AG_Netbuf *buf)
{
	SG_Quat q;

	q.w = SG_ReadReal(buf);
	q.x = SG_ReadReal(buf);
	q.y = SG_ReadReal(buf);
	q.z = SG_ReadReal(buf);
	return (q);
}

void
SG_ReadQuatv(AG_Netbuf *buf, SG_Quat *q)
{
	q->w = SG_ReadReal(buf);
	q->x = SG_ReadReal(buf);
	q->y = SG_ReadReal(buf);
	q->z = SG_ReadReal(buf);
}

void
SG_WriteQuat(AG_Netbuf *buf, SG_Quat *q)
{
	SG_WriteReal(buf, q->w);
	SG_WriteReal(buf, q->x);
	SG_WriteReal(buf, q->y);
	SG_WriteReal(buf, q->z);
}

#endif /* HAVE_OPENGL */
