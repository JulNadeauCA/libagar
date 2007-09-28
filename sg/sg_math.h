/*	Public domain	*/
/*
 * Basic mathematical structures, constants and functions.
 */

#ifdef _AGAR_INTERNAL
#include <config/have_math_fma.h>
#else
#include <agar/config/have_math_fma.h>
#endif

#ifdef SG_DOUBLE_PRECISION
typedef double SG_Real;
#define SG_REAL(n) AG_DOUBLE(n)
#else
typedef float SG_Real;
#define SG_REAL(n) AG_FLOAT(n)
#endif

typedef struct sg_range { SG_Real a, d, b; } SG_Range;
typedef struct sg_complex { SG_Real r, i; } SG_Complex;
typedef struct sg_quat { SG_Real w, x, y, z; } SG_Quat;
typedef struct sg_vector2 { SG_Real x, y; } SG_Vector2;
typedef struct sg_vector3 { SG_Real x, y, z; } SG_Vector3, SG_Vector;
typedef struct sg_vector4 { SG_Real x, y, z, w; } SG_Vector4;
typedef struct sg_line2 { SG_Vector2 p, d; SG_Real t; } SG_Line2;
typedef struct sg_line3 { SG_Vector p, d; SG_Real t; } SG_Line3, SG_Line;
typedef struct sg_plane { SG_Real a, b, c, d; } SG_Plane;
typedef struct sg_ray { SG_Vector p, dir; } SG_Ray;

enum sg_intersect_type {
	SG_NONE,
	SG_POINT,
	SG_LINE,
	SG_LINE_SEGMENT,
	SG_PLANE
};

typedef struct sg_intersect2 {
	enum sg_intersect_type type;
	union {
		SG_Vector2 ix_p;
		SG_Line2 ix_L;
	} ix;
} SG_Intersect2;

typedef struct sg_intersect {
	enum sg_intersect_type type;
	union {
		SG_Vector ix_p;
		SG_Line ix_L;
		SG_Plane ix_P;
	} ix;
} SG_Intersect3, SG_Intersect;

typedef struct sg_spherical {
	SG_Real phi;		/* Azimuth (longitude) */
	SG_Real theta;		/* Zenith (colatitude) */
	SG_Real r;		/* Radius */
} SG_Spherical;

#ifdef _AGAR_INTERNAL
#define ix_p ix.ix_p
#define ix_L ix.ix_L
#define ix_P ix.ix_P
#define ix_l ix.ix_l
#endif

#define SGVECTOR2(v) ((SG_Vector2 *)(v))
#define SGVECTOR3(v) ((SG_Vector3 *)(v))
#define SGVECTOR4(v) ((SG_Vector4 *)(v))
#define SGVECTOR(v) ((SG_Vector *)(v))
#define SGLINE2(v) ((SG_Line2 *)(v))
#define SGLINE3(v) ((SG_Line3 *)(v))
#define SGLINE(v) ((SG_Line *)(v))
#define SGPLANE(v) ((SG_Plane *)(v))
#define SGRAY(v) ((SG_Ray *)(v))

/*
 * Mathematical constants
 */
#define	SG_E		2.7182818284590452354	/* e */
#define	SG_LOG2E	1.4426950408889634074	/* log 2e */
#define	SG_LOG10E	0.43429448190325182765	/* log 10e */
#define	SG_LN2		0.69314718055994530942	/* log e2 */
#define	SG_LN10		2.30258509299404568402	/* log e10 */
#define	SG_PI		3.14159265358979323846	/* pi */
#define	SG_PI_2		1.57079632679489661923	/* pi/2 */
#define	SG_PI_4		0.78539816339744830962	/* pi/4 */
#define	SG_1_PI		0.31830988618379067154	/* 1/pi */
#define	SG_2_PI		0.63661977236758134308	/* 2/pi */
#define	SG_2_SQRTPI	1.12837916709551257390	/* 2/sqrt(pi) */
#define	SG_SQRT2	1.41421356237309504880	/* sqrt(2) */
#define	SG_SQRT1_2	0.70710678118654752440	/* 1/sqrt(2) */

/*
 * Basic math routines
 */
#ifdef SG_DOUBLE_PRECISION

#define SG_Sqrt(x) sqrt(x)
#define SG_Cbrt(x) cbrt(x)
#define SG_Sin(x) sin(x)
#define SG_Cos(x) cos(x)
#define SG_Tan(x) tan(x)
#define SG_Cot(x) (1.0/tan(x))
#define SG_Sec(x) (1.0/cos(x))
#define SG_Csc(x) (1.0/sin(x))
#define SG_Asin(x) asin(x)
#define SG_Acos(x) acos(x)
#define SG_Atan(x) atan(x)
#define SG_Atan2(y,x) atan2((y),(x))
#define SG_Hypot2(x,y) hypot((x),(y))
#define SG_Fabs(x) fabs(x)
#define SG_Pow(x,y) pow((x),(y))

#ifdef HAVE_MATH_FMA
#define SG_Fma(x,y,z) fma((x),(y),(z))
#else
#define SG_Fma(x,y,z) SG_FmaEmul((x),(y),(z))
#endif

#else /* !SG_DOUBLE_PRECISION */

#define SG_Sqrt(r) sqrtf(r)
#define SG_Cbrt(r) cbrtf(r)
#define SG_Sin(x) sinf(x)
#define SG_Cos(x) cosf(x)
#define SG_Tan(x) tanf(x)
#define SG_Cot(x) (1.0/tanf(x))
#define SG_Sec(x) (1.0/cosf(x))
#define SG_Csc(x) (1.0/sinf(x))
#define SG_Asin(x) asinf(x)
#define SG_Acos(x) acosf(x)
#define SG_Atan(x) atanf(x)
#define SG_Atan2(y,x) atan2f((y),(x))
#define SG_Hypot2(x,y) hypotf((x),(y))
#define SG_Fabs(x) fabsf(x)
#define SG_Pow(x,y) powf((x),(y))

#ifdef HAVE_MATH_FMA
#define SG_Fma(x,y,z) fmaf((x),(y),(z))
#else
#define SG_Fma(x,y,z) SG_FmaEmulf((x),(y),(z))
#endif

#endif /* !SG_DOUBLE_PRECISION */

#define SG_Radians(x) ((x)/360.0*2.0*SG_PI)
#define SG_Degrees(x) (360.0*((x)/(2.0*SG_PI)))
#define SG_Sgn(x) (((x)<0.0)?-1.0 : ((x)>0.0)?1.0 : 0.0)

#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_MATH)
#define Sqrt(r) SG_Sqrt(r)
#define Cbrt(r) SG_Cbrt(r)
#define Sin(x) SG_Sin(x)
#define Cos(x) SG_Cos(x)
#define Tan(x) SG_Tan(x)
#define Cot(x) SG_Cot(x)
#define Sec(x) SG_Sec(x)
#define Csc(x) SG_Csc(x)
#define Asin(x) SG_Asin(x)
#define Acos(x) SG_Acos(x)
#define Atan(x) SG_Atan(x)
#define Atan2(y,x) SG_Atan2((y),(x))
#define Hypot2(x,y) SG_Hypot2((x),(y))
#define Fabs(x) SG_Fabs(x)
#define Pow(x,y) SG_Pow((x),(y))
#define Sgn(x) SG_Sgn((x))
#define Radians(x) SG_Radians((x))
#define Degrees(x) SG_Degrees((x))
#define Sgn(x) SG_Sgn((x))
#endif /* _AGAR_INTERNAL */

__BEGIN_DECLS
__inline__ SG_Real	SG_Rad2Deg(SG_Real);
__inline__ SG_Real	SG_Deg2Rad(SG_Real);
__inline__ SG_Spherical	SG_CartToSph(SG_Vector);
__inline__ SG_Vector	SG_SphToCart(SG_Spherical);

SG_Real SG_ReadReal(AG_Netbuf *);
void	SG_CopyReal(AG_Netbuf *, SG_Real *);
void	SG_WriteReal(AG_Netbuf *, SG_Real);
__END_DECLS
