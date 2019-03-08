/*	Public domain	*/

#ifndef _AGAR_VG_MATH_H_
#define _AGAR_VG_MATH_H_

#include <string.h>

#include <agar/config/have_math.h>
#include <agar/config/have_math_c99.h>
#ifdef HAVE_MATH
# include <math.h>
#endif

#ifdef M_PI
# define VG_PI M_PI
#else
# define VG_PI 3.14159265358979323846
#endif

#ifdef HAVE_MATH_C99
# define VG_Sin(x) sinf(x)
# define VG_Cos(x) cosf(x)
# define VG_Tan(x) tanf(x)
# define VG_Mod(x,y) fmodf((x),(y))
# define VG_Sqrt(x) sqrtf(x)
# define VG_Atan2(y,x) atan2f((y),(x))
# define VG_Floor(x) floorf(x)
# define VG_Ceil(x) ceilf(x)
# define VG_Fabs(x) fabsf(x)
# define VG_Hypot(x,y) hypotf((x),(y))
# define VG_Rint(x) rintf(x)
#else
# define VG_Sin(x) ((float)sin((double)x))
# define VG_Cos(x) ((float)cos((double)x))
# define VG_Tan(x) ((float)tan((double)x))
# define VG_Mod(x,y) ((float)fmod((double)(x),(double)(y)))
# define VG_Sqrt(x) ((float)sqrt((double)x))
# define VG_Atan2(y,x) ((float)atan2((double)(y),(double)(x)))
# define VG_Floor(x) ((float)floor((double)x))
# define VG_Ceil(x) ((float)ceil((double)x))
# define VG_Fabs(x) ((float)fabs((double)x))
# define VG_Hypot(x,y) ((float)hypot((double)(x),(double)(y)))
# define VG_Rint(x) VG_Floor((x) + 0.5f)
#endif /* HAVE_MATH_C99 */

#define VG_Degrees(x) ((x)/(2.0*VG_PI)*360.0)
#define VG_Radians(x) (((x)/360.0)*(2.0*VG_PI))
#define VGVECTOR(x,y) VG_GetVector((x),(y))

#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_VG_MATH)
# define Sin(x) VG_Sin(x)
# define Cos(x) VG_Cos(x)
# define Tan(x) VG_Tan(x)
# define Mod(x,y) VG_Mod((x),(y))
# define Sqrt(x) VG_Sqrt(x)
# define Atan2(y,x) VG_Atan2((y),(x))
# define Floor(x) VG_Floor(x)
# define Ceil(x) VG_Ceil(x)
# define Fabs(x) VG_Fabs(x)
# define Hypot(x) VG_Hypot(x)
#endif /* _AGAR_INTERNAL or _USE_AGAR_VG_MATH */

__BEGIN_DECLS
extern int vg_cos_tbl[];
extern int vg_sin_tbl[];

/*
 * Basic vector operations
 */
static __inline__ VG_Vector
VG_GetVector(float x, float y)
{
	VG_Vector v;
	v.x = x;
	v.y = y;
	return (v);
}
static __inline__ VG_Vector
VG_Add(VG_Vector v1, VG_Vector v2)
{
	VG_Vector v3;
	v3.x = v1.x + v2.x;
	v3.y = v1.y + v2.y;
	return (v3);
}
static __inline__ VG_Vector
VG_Sub(VG_Vector v1, VG_Vector v2)
{
	VG_Vector v3;
	v3.x = v1.x - v2.x;
	v3.y = v1.y - v2.y;
	return (v3);
}
static __inline__ VG_Vector
VG_ScaleVector(float a, VG_Vector v)
{
	VG_Vector av;
	av.x = a*v.x;
	av.y = a*v.y;
	return (av);
}
static __inline__ float
VG_DotProd(VG_Vector v1, VG_Vector v2)
{
	return (v1.x*v2.x + v1.y*v2.y);
}
static __inline__ float
VG_Length(VG_Vector v)
{
	return VG_Sqrt(VG_DotProd(v,v));
}
static __inline__ float
VG_Distance(VG_Vector v1, VG_Vector v2)
{
	return VG_Length(VG_Sub(v1,v2));
}

/* Compute minimal point-line distance. */
static __inline__ float
VG_PointLineDistance(VG_Vector A, VG_Vector B, VG_Vector *_Nonnull pt)
{
	float mag, u;
	VG_Vector vInt;

	mag = VG_Distance(B, A);
	u = ((pt->x - A.x)*(B.x - A.x) + (pt->y - A.y)*(B.y - A.y))/(mag*mag);
	if (u < 0.0f) {
		vInt = A;
	} else if (u > 1.0f) {
		vInt = B;
	} else {
		vInt.x = A.x + u*(B.x - A.x);
		vInt.y = A.y + u*(B.y - A.y);
	}
	mag = VG_Distance(*pt, vInt);
	pt->x = vInt.x;
	pt->y = vInt.y;
	return (mag);
}

/*
 * Compute the intersection point between a given line (v1,v2) and a
 * vertical line at X.
 */
static __inline__ VG_Vector
VG_IntersectLineV(float x, VG_Vector v1, VG_Vector v2)
{
	float x1 = v1.x, x2 = v2.x;
	float y1 = v1.y, y2 = v2.y;
	float m, x3 = x1;
	VG_Vector v;

	if (y1 < y2) { m = y1; y1 = y2; y2 = m; x3 = x2; }
	if (x1 < x2) { m = x1; x1 = x2; x2 = m; }
	m = VG_Fabs(y2-y1)/VG_Fabs(x2-x1);
	v.x = x;
	v.y = m*(x - x3);
	return (v);
}

/*
 * Compute the intersection point between a given line (v1,v2) and a
 * horizontal line at Y.
 */
static __inline__ VG_Vector
VG_IntersectLineH(float y, VG_Vector v1, VG_Vector v2)
{
	float x1 = v1.x, x2 = v2.x;
	float y1 = v1.y, y2 = v2.y;
	float m, y3 = y1;
	VG_Vector v;

	if (y1 < y2) { m = y1; y1 = y2; y2 = m; }
	if (x1 < x2) { m = x1; x1 = x2; x2 = m; y3 = y2; }
	m = VG_Fabs(y2-y1)/VG_Fabs(x2-x1);
	v.x = m*(y - y3);
	v.y = y;
	return (v);
}

/*
 * Basic matrix operations.
 */
static __inline__ VG_Matrix
VG_MatrixIdentity(void)
{
	VG_Matrix T;
	T.m[0][0] = 1.0; T.m[0][1] = 0.0; T.m[0][2] = 0.0;
	T.m[1][0] = 0.0; T.m[1][1] = 1.0; T.m[1][2] = 0.0;
	T.m[2][0] = 0.0; T.m[2][1] = 0.0; T.m[2][2] = 1.0;
	return (T);
}
static __inline__ void
VG_MultMatrix(VG_Matrix *_Nonnull T, const VG_Matrix *_Nonnull A)
{
	VG_Matrix R;
	int m, n;

	for (m = 0; m < 3; m++) {
		for (n = 0; n < 3; n++)
			R.m[m][n] = T->m[m][0]*A->m[0][n] +
			            T->m[m][1]*A->m[1][n] +
			            T->m[m][2]*A->m[2][n];
	}
	memcpy(T, &R, sizeof(VG_Matrix));
}
static __inline__ void
VG_MultMatrixByVector(VG_Vector *_Nonnull c, const VG_Vector *_Nonnull a,
    const VG_Matrix *_Nonnull T)
{
	float ax = a->x;
	float ay = a->y;

	c->x = ax*T->m[0][0] + ay*T->m[1][0] + T->m[0][2];
	c->y = ax*T->m[0][1] + ay*T->m[1][1] + T->m[1][2];
}
__END_DECLS

#endif /* _AGAR_VG_MATH_H_ */
