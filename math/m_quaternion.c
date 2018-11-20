/*
 * Copyright (c) 2005-2008 Hypertriton, Inc. <http://hypertriton.com/>
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

#include <agar/core/core.h>
#include <agar/math/m.h>

#define SLERP_TO_LERP_THRESH 0.001

/* Multiplicative Identity */
M_Quaternion
M_QuaternionMultIdentity(void)
{
	M_Quaternion q;

	q.w = 1.0;
	q.x = 0.0;
	q.y = 0.0;
	q.z = 0.0;
	return (q);
}

/* Additive Identity */
M_Quaternion
M_QuaternionAddIdentity(void)
{
	M_Quaternion q;

	q.w = 0.0;
	q.x = 0.0;
	q.y = 0.0;
	q.z = 0.0;
	return (q);
}

void
M_QuaternionToMatrix44(M_Matrix44 *A, const M_Quaternion *q)
{
	M_Real x = q->x;
	M_Real y = q->y;
	M_Real z = q->z;
	M_Real w = q->w;

	A->m[0][0] = 1.0 - 2.0*y*y - 2.0*z*z;
	A->m[0][1] =       2.0*x*y - 2.0*w*z;
	A->m[0][2] =       2.0*x*z + 2.0*w*y;
	A->m[0][3] = 0.0;

	A->m[1][0] =       2.0*x*y + 2.0*w*z;
	A->m[1][1] = 1.0 - 2.0*x*x - 2.0*z*z;
	A->m[1][2] =       2.0*y*z - 2.0*w*x;
	A->m[1][3] = 0.0;

	A->m[2][0] =       2.0*x*z - 2.0*w*y;
	A->m[2][1] =       2.0*y*z + 2.0*w*x;
	A->m[2][2] = 1.0 - 2.0*x*x - 2.0*y*y;
	A->m[2][3] = 0.0;
	
	A->m[3][0] = 0.0;
	A->m[3][1] = 0.0;
	A->m[3][2] = 0.0;
	A->m[3][3] = 1.0;
}

void
M_QuaternionpToAxisAngle(const M_Quaternion *q, M_Vector3 *v, M_Real *theta)
{
	M_Real s;
	
	s = Sqrt(q->x*q->x + q->y*q->y + q->z*q->z);
	if (s != 0.0) {
		*theta = 2.0*Acos(q->w);
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
M_QuaternionpToAxisAngle3(const M_Quaternion *q, M_Real *theta, M_Real *x,
    M_Real *y, M_Real *z)
{
	M_Vector3 axis;
	
	M_QuaternionpToAxisAngle(q, &axis, theta);
	*x = axis.x;
	*y = axis.y;
	*z = axis.z;
}

M_Quaternion
M_QuaternionFromAxisAngle(M_Vector3 axis, M_Real theta)
{
	M_Quaternion q;
	M_Real s;

	s = Sin(theta/2.0);
	q.w = Cos(theta/2.0);
	q.x = axis.x*s;
	q.y = axis.y*s;
	q.z = axis.z*s;
	M_QuaternionNormv(&q);
	return (q);
}

M_Quaternion
M_QuaternionFromAxisAngle3(M_Real theta, M_Real x, M_Real y, M_Real z)
{
	M_Quaternion q;
	M_Real s;

	s = Sin(theta/2.0);
	q.w = Cos(theta/2.0);
	q.x = x*s;
	q.y = y*s;
	q.z = z*s;
	M_QuaternionNormv(&q);
	return (q);
}

void
M_QuaternionpFromAxisAngle(M_Quaternion *q, M_Vector3 axis, M_Real theta)
{
	M_Real s;

	s = Sin(theta/2.0);
	q->w = Cos(theta/2.0);
	q->x = axis.x*s;
	q->y = axis.y*s;
	q->z = axis.z*s;
	M_QuaternionNormv(q);
}

void
M_QuaternionpFromAxisAngle3(M_Quaternion *q, M_Real theta, M_Real x, M_Real y,
    M_Real z)
{
	M_Real s, len;

	s = Sin(theta/2.0);
	q->w = Cos(theta/2.0);
	q->x = x*s;
	q->y = y*s;
	q->z = z*s;
	len = Sqrt(q->x*q->x + q->y*q->y + q->z*q->z + q->w*q->w);
	q->w /= len;
	q->x /= len;
	q->y /= len;
	q->z /= len;
}

void
M_QuaternionFromEulv(M_Quaternion *Qr, M_Real a, M_Real b, M_Real c)
{
	M_Quaternion Qx, Qy;

	Qx.w = Cos(a/2.0);
	Qx.x = Sin(a/2.0);
	Qx.y = 0.0;
	Qx.z = 0.0;

	Qy.w = Cos(b/2.0);
	Qy.x = 0.0;
	Qy.y = Sin(b/2.0);
	Qy.z = 0.0;
	
	Qy.w = Cos(c/2.0);
	Qy.x = 0.0;
	Qy.y = 0.0;
	Qy.z = Sin(b/2.0);

	*Qr = M_QuaternionMult3(Qx, Qy, Qx);
}

M_Quaternion
M_QuaternionFromEul(M_Real a, M_Real b, M_Real c)
{
	M_Quaternion Qr;

	M_QuaternionFromEulv(&Qr, a, b, c);
	return (Qr);
}

M_Quaternion
M_QuaternionConj(M_Quaternion q)
{
	return (M_QuaternionConjp(&q));
}

M_Quaternion
M_QuaternionConjp(const M_Quaternion *q)
{
	M_Quaternion nq;

	nq.x = -(q->x);
	nq.y = -(q->y);
	nq.z = -(q->z);
	nq.w = q->w;
	return (nq);
}

void
M_QuaternionConjv(M_Quaternion *q)
{
	q->x = -(q->x);
	q->y = -(q->y);
	q->z = -(q->z);
}

M_Quaternion
M_QuaternionScale(M_Quaternion q, M_Real c)
{
	return (M_QuaternionScalep(&q, c));
}

M_Quaternion
M_QuaternionScalep(const M_Quaternion *q, M_Real c)
{
	M_Quaternion nq;

	nq.x = q->x*c;
	nq.y = q->y*c;
	nq.z = q->z*c;
	nq.w = q->w*c;
	return (nq);
}

void
M_QuaternionScalev(M_Quaternion *q, M_Real c)
{
	q->x *= c;
	q->y *= c;
	q->z *= c;
	q->w *= c;
}

M_Quaternion
M_QuaternionMult(M_Quaternion q1, M_Quaternion q2)
{
	return (M_QuaternionMultp(&q1, &q2));
}

M_Quaternion
M_QuaternionMultp(const M_Quaternion *A, const M_Quaternion *B)
{
	M_Quaternion C;

	C.w = A->w*B->w - A->x*B->x - A->y*B->y - A->z*B->z;
	C.x = A->w*B->x + A->x*B->w + A->y*B->z - A->z*B->y;
	C.y = A->w*B->y - A->x*B->z + A->y*B->w + A->z*B->x;
	C.z = A->w*B->z + A->x*B->y - A->y*B->x + A->z*B->w;
	return (C);
}

void
M_QuaternionMultv(M_Quaternion *C, const M_Quaternion *A, const M_Quaternion *B)
{
	C->w = A->w*B->w - A->x*B->x - A->y*B->y - A->z*B->z;
	C->x = A->w*B->x + A->x*B->w + A->y*B->z - A->z*B->y;
	C->y = A->w*B->y - A->x*B->z + A->y*B->w + A->z*B->x;
	C->z = A->w*B->z + A->x*B->y - A->y*B->x + A->z*B->w;
}

/* Alternative to M_QuaternionMult(). */
M_Quaternion
M_QuaternionConcat(const M_Quaternion *A, const M_Quaternion *B)
{
	M_Vector3 v1, v2, cross;
	M_Quaternion R;
	M_Real angle;

	v1.x = A->x;
	v1.y = A->y;
	v1.z = A->z;
	v2.x = B->x;
	v2.y = B->y;
	v2.z = B->z;
	angle = A->w*B->w - M_VecDot3p(&v1, &v2);
	cross = M_VecCross3p(&v1, &v2);
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

M_Quaternion
M_QuaternionNorm(M_Quaternion q)
{
	return (M_QuaternionNormp(&q));
}

M_Quaternion
M_QuaternionNormp(const M_Quaternion *q)
{
	M_Quaternion nq;
	M_Real s;

	s = Sqrt(q->x*q->x + q->y*q->y + q->z*q->z + q->w*q->w);
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
M_QuaternionNormv(M_Quaternion *q)
{
	M_Real s;

	s = Sqrt(q->x*q->x + q->y*q->y + q->z*q->z + q->w*q->w);
	if (s == 0.0) {
		return;
	}
	q->x /= s;
	q->y /= s;
	q->z /= s;
	q->w /= s;
}

M_Quaternion
M_QuaternionInverse(M_Quaternion Q)
{
	M_Quaternion Qc;

	Qc = M_QuaternionConjp(&Q);
	M_QuaternionNormv(&Qc);
	return (Qc);
}

M_Quaternion
M_QuaternionInversep(const M_Quaternion *Q)
{
	M_Quaternion Qc;

	Qc = M_QuaternionConjp(Q);
	M_QuaternionNormv(&Qc);
	return (Qc);
}

void
M_QuaternionInversev(M_Quaternion *Q)
{
	M_QuaternionConjv(Q);
	M_QuaternionNormv(Q);
}

M_Quaternion
M_QuaternionSLERP(M_Quaternion q1, M_Quaternion q2, M_Real alpha)
{
	return (M_QuaternionSLERPp(&q1, &q2, alpha));
}

M_Quaternion
M_QuaternionSLERPp(const M_Quaternion *q1, const M_Quaternion *q2,
    M_Real alpha)
{
	M_Real o, co, so, scale0, scale1;
	M_Real qi[4];
	M_Quaternion qr;

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

	if ((1.0 - co) < SLERP_TO_LERP_THRESH) {
		scale0 = 1.0-alpha;
		scale1 = alpha;
	} else {
		o = Acos(co);
		so = Sin(o);
		scale0 = Sin((1.0-alpha)*o)/so;
		scale1 = Sin(alpha*o)/so;
	}

	qr.w = scale0*q1->w + scale1*qi[3];
	qr.x = scale0*q1->x + scale1*qi[0];
	qr.y = scale0*q1->y + scale1*qi[1];
	qr.z = scale0*q1->z + scale1*qi[2];
	return (qr);
}

M_Quaternion
M_ReadQuaternion(AG_DataSource *buf)
{
	M_Quaternion q;

	q.w = M_ReadReal(buf);
	q.x = M_ReadReal(buf);
	q.y = M_ReadReal(buf);
	q.z = M_ReadReal(buf);
	return (q);
}

void
M_WriteQuaternion(AG_DataSource *buf, M_Quaternion *q)
{
	M_WriteReal(buf, q->w);
	M_WriteReal(buf, q->x);
	M_WriteReal(buf, q->y);
	M_WriteReal(buf, q->z);
}
