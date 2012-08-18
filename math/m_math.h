/*	Public domain	*/
/*
 * Basic mathematical structures, constants and functions.
 */

#include <agar/core/limits.h>

/* Standard math library */
#include <agar/config/have_math.h>
#ifdef HAVE_MATH
# include <math.h>
#endif

/* AltiVec extensions */
#include <agar/config/have_altivec.h>
#include <agar/config/have_altivec_h.h>
#include <agar/config/inline_altivec.h>
#if defined(HAVE_ALTIVEC) && defined(HAVE_ALTIVEC_H)
# include <altivec.h>
#endif

/* SSE extensions */
#include <agar/config/have_sse.h>
#include <agar/config/have_sse2.h>
#include <agar/config/have_sse3.h>
#include <agar/config/inline_sse.h>
#ifdef HAVE_SSE
# include <xmmintrin.h>
#endif
#ifdef HAVE_SSE2
# include <emmintrin.h>
#endif
#ifdef HAVE_SSE3
# include <pmmintrin.h>
#endif

/*
 * Real numbers, complex numbers and linear algebra structures.
 */

#if defined(QUAD_PRECISION)
typedef long double M_Real;
# define M_REAL(n)		AG_LONG_DOUBLE(n)
# define M_VARIABLE_REAL	AG_VARIABLE_LONG_DOUBLE
# define M_VARIABLE_P_REAL	AG_VARIABLE_P_LONG_DOUBLE
# define M_SetReal		AG_SetLongDouble
# define M_GetReal		AG_GetLongDouble
# define M_BindReal		AG_BindLongDouble
# define M_BindRealFn		AG_BindLongDoubleFn
#elif defined(DOUBLE_PRECISION)
typedef double M_Real;
# define M_REAL(n)		AG_DOUBLE(n)
# define M_VARIABLE_REAL	AG_VARIABLE_DOUBLE
# define M_VARIABLE_P_REAL	AG_VARIABLE_P_DOUBLE
# define M_SetReal		AG_SetDouble
# define M_GetReal		AG_GetDouble
# define M_BindReal		AG_BindDouble
# define M_BindRealFn		AG_BindDoubleFn
#elif defined(SINGLE_PRECISION)
typedef float M_Real;
# define M_REAL(n)		AG_FLOAT(n)
# define M_VARIABLE_REAL	AG_VARIABLE_FLOAT
# define M_VARIABLE_P_REAL	AG_VARIABLE_P_FLOAT
# define M_SetReal		AG_SetFloat
# define M_GetReal		AG_GetFloat
# define M_BindReal		AG_BindFloat
# define M_BindRealFn		AG_BindFloatFn
#else
# error "Math precision not specified"
#endif

typedef M_Real M_Time;
#define M_VARIABLE_TIME	 	 M_VARIABLE_REAL
#define M_VARIABLE_P_TIME	 M_VARIABLE_P_REAL
#define M_SetTime		 M_SetReal
#define M_GetTime		 M_GetReal
#define M_BindTime		 M_BindReal
#define M_BindTimeFn		 M_BindRealFn

typedef struct m_range { M_Real v, min, max, typ; } M_Range;
typedef struct m_time_range { M_Time v, min, max, typ; } M_TimeRange;
typedef struct m_complex { M_Real r, i; } M_Complex;
typedef struct m_quat { M_Real w, x, y, z; } M_Quaternion;
typedef struct m_vector2 { M_Real x, y; } M_Vector2;
typedef struct m_rectangular { M_Real x, y, z; } M_Rectangular;
typedef struct m_polar { M_Real theta, r; } M_Polar;
typedef struct m_parabolic { M_Real eta, xi, phi; } M_Parabolic;
typedef struct m_spherical { M_Real phi, theta, r; } M_Spherical;
typedef struct m_cylindrical { M_Real rho, phi, z; } M_Cylindrical;

#if defined(HAVE_SSE)

typedef union m_vector3 {
	__m128 m128;
	struct { float x, y, z, _pad; };
} M_Vector3;
typedef union m_vector4 {
	__m128 m128;
	struct { float x, y, z, w; };
} M_Vector4;
typedef union m_matrix44 {
	struct { __m128 m1, m2, m3, m4; };
	float m[4][4];
} M_Matrix44;
typedef union m_color {
	__m128 m128;
	struct { float r, g, b, a; };
} M_Color;

#else /* !HAVE_SSE */

typedef struct m_vector3 { M_Real x, y, z; } M_Vector3;
typedef struct m_vector4 { M_Real x, y, z, w; } M_Vector4;
typedef struct m_matrix44 { M_Real m[4][4]; } M_Matrix44;
typedef struct m_color { M_Real r, g, b, a; } M_Color;

#endif /* HAVE_SSE */

/* Vector in R^n. */
typedef struct m_vector {
	Uint m;			/* Size */
	M_Real *v;		/* Elements */
} M_Vector;

struct m_matrix_ops;

/* Base class for m-by-n matrices. */
typedef struct m_matrix {
	const struct m_matrix_ops *ops;
	Uint m;			/* Rows */
	Uint n;			/* Columns */
} M_Matrix;

#define MVECTOR(v) ((M_Vector *)(v))
#define MVECTOR2(v) ((M_Vector2 *)(v))
#define MVECTOR3(v) ((M_Vector3 *)(v))
#define MVECTOR4(v) ((M_Vector4 *)(v))
#define MVECSIZE(v) (((M_Vector *)(v))->m)

#define MMATRIX(v) ((M_Matrix *)(v))
#define MMATRIX44(v) ((M_Matrix44 *)(v))
#define MROWS(v) (((M_Matrix *)(v))->m)
#define MCOLS(v) (((M_Matrix *)(v))->n)

/*
 * Mathematical constants
 */
#undef M_E
#undef M_LOG2E
#undef M_LOG10E
#undef M_LN2
#undef M_LN10
#undef M_PI
#undef M_PI_2
#undef M_PI_4
#undef M_1_PI
#undef M_2_PI
#undef M_2_SQRTPI
#undef M_SQRT2
#undef M_SQRT1_2
#if defined(QUAD_PRECISION)
# define M_E		2.7182818284590452353602874713526625L  /* e */
# define M_LOG2E	1.4426950408889634073599246810018921L  /* log_2 e */
# define M_LOG10E	0.4342944819032518276511289189166051L  /* log_10 e */
# define M_LN2		0.6931471805599453094172321214581766L  /* log_e 2 */
# define M_LN10		2.3025850929940456840179914546843642L  /* log_e 10 */
# define M_PI		3.1415926535897932384626433832795029L  /* pi */
# define M_PI_2		1.5707963267948966192313216916397514L  /* pi/2 */
# define M_PI_4		0.7853981633974483096156608458198757L  /* pi/4 */
# define M_1_PI		0.3183098861837906715377675267450287L  /* 1/pi */
# define M_2_PI		0.6366197723675813430755350534900574L  /* 2/pi */
# define M_2_SQRTPI	1.1283791670955125738961589031215452L  /* 2/sqrt(pi) */
# define M_SQRT2	1.4142135623730950488016887242096981L  /* sqrt(2) */
# define M_SQRT1_2	0.7071067811865475244008443621048490L  /* 1/sqrt(2) */
#else
# define M_E		2.7182818284590452354	/* e */
# define M_LOG2E	1.4426950408889634074	/* log 2e */
# define M_LOG10E	0.43429448190325182765	/* log 10e */
# define M_LN2		0.69314718055994530942	/* log e2 */
# define M_LN10		2.30258509299404568402	/* log e10 */
# define M_PI		3.14159265358979323846	/* pi */
# define M_PI_2		1.57079632679489661923	/* pi/2 */
# define M_PI_4		0.78539816339744830962	/* pi/4 */
# define M_1_PI		0.31830988618379067154	/* 1/pi */
# define M_2_PI		0.63661977236758134308	/* 2/pi */
# define M_2_SQRTPI	1.12837916709551257390	/* 2/sqrt(pi) */
# define M_SQRT2	1.41421356237309504880	/* sqrt(2) */
# define M_SQRT1_2	0.70710678118654752440	/* 1/sqrt(2) */
#endif

/*
 * Floating point memory format parameters.
 */
#undef M_EXPMIN
#undef M_EXPMAX
#undef M_PRECISION
#undef M_PRECISION_2
#undef M_NUMMAX
#undef M_MACHEP
#undef M_INFINITY
#if defined(QUAD_PRECISION)
# define M_EXPMIN	-16382
# define M_EXPMAX	 16383
# define M_PRECISION	 113
# define M_PRECISION_2	 57
# define M_NUMMAX	 AG_LDBL_MAX
# define M_MACHEP	 5.42101086242752217003726400434970855712890625E-20l
# define M_TINYVAL	 5.42101086242752217003726400434970855712890625E-19l
# define M_HUGEVAL	 1.1e+4931l
#elif defined(DOUBLE_PRECISION)
# define M_EXPMIN	-1022
# define M_EXPMAX	 1023
# define M_PRECISION	 53
# define M_PRECISION_2	 27
# define M_NUMMAX	 AG_DBL_MAX
# define M_MACHEP	 1.1e-16
# define M_TINYVAL	 1.1e-15
# define M_HUGEVAL	 1.7e+307
#elif defined(SINGLE_PRECISION)
# define M_EXPMIN	-126
# define M_EXPMAX	 127
# define M_PRECISION	 24
# define M_PRECISION_2	 12
# define M_NUMMAX	 AG_FLT_MAX
# define M_MACHEP	 3.0e-8f
# define M_TINYVAL	 3.0e-7f
# define M_HUGEVAL	 3.4e+37f
#endif
#define M_INFINITY HUGE_VAL
#define M_MACHZERO(n) ((n) >= -M_MACHEP && (n) <= +M_MACHEP)

/*
 * Standard math library routines
 */
#if defined(QUAD_PRECISION)

# define M_Log(x) logl(x)
# define M_Exp(x) expl(x)
# define M_ExpM1(x) expm1l(x)
# define M_Sqrt(x) sqrtl(x)
# define M_Cbrt(x) cbrtl(x)
# define M_Sin(x) sinl(x)
# define M_Cos(x) cosl(x)
# define M_Tan(x) tanl(x)
# define M_Sinh(x) sinhl(x)
# define M_Cosh(x) coshl(x)
# define M_Tanh(x) tanhl(x)
# define M_Cot(x) (1.0l/tanl(x))
# define M_Sec(x) (1.0l/cosl(x))
# define M_Csc(x) (1.0l/sinl(x))
# define M_Asin(x) asinl(x)
# define M_Acos(x) acosl(x)
# define M_Atan(x) atanl(x)
# define M_Asinh(x) asinhl(x)
# define M_Acosh(x) acoshl(x)
# define M_Atanh(x) atanhl(x)
# define M_Atan2(y,x) atan2l((y),(x))
# define M_Hypot2(x,y) hypotl((x),(y))
# define M_Fabs(x) fabsl(x)
# define M_Sgn(x) ((x)>0.0l ? 1.0l : ((x)<0.0l ? -1.0l : 0.0l))
# define M_Pow(x,y) powl((x),(y))
# define M_Frexp(x,exp) frexpl((x),(exp))
# define M_Ldexp(x,exp) ldexpl((x),(exp))
# define M_Ceil(x) ceill(x)
# define M_Floor(x) floorl(x)
# define M_IsNaN(x) isnanl(x)
# define M_IsInf(x) isinfl(x)

#elif defined(DOUBLE_PRECISION)

# define M_Log(x) log(x)
# define M_Exp(x) exp(x)
# define M_ExpM1(x) expm1(x)
# define M_Sqrt(x) sqrt(x)
# define M_Cbrt(x) cbrt(x)
# define M_Sin(x) sin(x)
# define M_Cos(x) cos(x)
# define M_Tan(x) tan(x)
# define M_Sinh(x) sinh(x)
# define M_Cosh(x) cosh(x)
# define M_Tanh(x) tanh(x)
# define M_Cot(x) (1.0/tan(x))
# define M_Sec(x) (1.0/cos(x))
# define M_Csc(x) (1.0/sin(x))
# define M_Asin(x) asin(x)
# define M_Acos(x) acos(x)
# define M_Atan(x) atan(x)
# define M_Asinh(x) asinh(x)
# define M_Acosh(x) acosh(x)
# define M_Atanh(x) atanh(x)
# define M_Atan2(y,x) atan2((y),(x))
# define M_Hypot2(x,y) hypot((x),(y))
# define M_Fabs(x) fabs(x)
# define M_Sgn(x) ((x)>0.0 ? 1.0 : ((x)<0.0 ? -1.0 : 0.0))
# define M_Pow(x,y) pow((x),(y))
# define M_Frexp(x,exp) frexp((x),(exp))
# define M_Ldexp(x,exp) ldexp((x),(exp))
# define M_Ceil(x) ceil(x)
# define M_Floor(x) floor(x)
# define M_IsNaN(x) isnan(x)
# define M_IsInf(x) isinf(x)

#elif defined(SINGLE_PRECISION)

# define M_Log(x) logf(x)
# define M_Exp(x) expf(x)
# define M_ExpM1(x) expm1f(x)
# define M_Sqrt(r) sqrtf(r)
# define M_Cbrt(r) cbrtf(r)
# define M_Sin(x) sinf(x)
# define M_Cos(x) cosf(x)
# define M_Tan(x) tanf(x)
# define M_Sinh(x) sinhf(x)
# define M_Cosh(x) coshf(x)
# define M_Tanh(x) tanhf(x)
# define M_Cot(x) (1.0f/tanf(x))
# define M_Sec(x) (1.0f/cosf(x))
# define M_Csc(x) (1.0f/sinf(x))
# define M_Asin(x) asinf(x)
# define M_Acos(x) acosf(x)
# define M_Atan(x) atanf(x)
# define M_Asinh(x) asinhf(x)
# define M_Acosh(x) acoshf(x)
# define M_Atanh(x) atanhf(x)
# define M_Atan2(y,x) atan2f((y),(x))
# define M_Hypot2(x,y) hypotf((x),(y))
# define M_Fabs(x) fabsf(x)
# define M_Sgn(x) ((x)>0.0f ? 1.0f : ((x)<0.0f ? -1.0f : 0.0f))
# define M_Pow(x,y) powf((x),(y))
# define M_Frexp(x,exp) frexpf((x),(exp))
# define M_Ldexp(x,exp) ldexpf((x),(exp))
# define M_Ceil(x) ceilf(x)
# define M_Floor(x) floorf(x)
# define M_IsNaN(x) isnanf(x)
# define M_IsInf(x) isinff(x)

#endif /* PRECISION */

#define M_Radians(x) ((x)/360.0*2.0*M_PI)
#define M_Degrees(x) (360.0*((x)/(2.0*M_PI)))
#define M_Max(h,i) ((h) > (i) ? (h) : (i))
#define M_Min(l,o) ((l) < (o) ? (l) : (o))

#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_MATH)
# define Log(r) M_Log(r)
# define Exp(r) M_Exp(r)
# define Sqrt(r) M_Sqrt(r)
# define Cbrt(r) M_Cbrt(r)
# define Sin(x) M_Sin(x)
# define Cos(x) M_Cos(x)
# define Tan(x) M_Tan(x)
# define Sinh(x) M_Sinh(x)
# define Cosh(x) M_Cosh(x)
# define Tanh(x) M_Tanh(x)
# define Cot(x) M_Cot(x)
# define Sec(x) M_Sec(x)
# define Csc(x) M_Csc(x)
# define Asin(x) M_Asin(x)
# define Acos(x) M_Acos(x)
# define Atan(x) M_Atan(x)
# define Asinh(x) M_Asinh(x)
# define Acosh(x) M_Acosh(x)
# define Atanh(x) M_Atanh(x)
# define Atan2(y,x) M_Atan2((y),(x))
# define Hypot2(x,y) M_Hypot2((x),(y))
# define Fabs(x) M_Fabs(x)
# define Pow(x,y) M_Pow((x),(y))
# define Frexp(x,exp) M_Frexp((x),(exp))
# define Ldexp(x,exp) M_Ldexp((x),(exp))
# define Ceil(x) M_Ceil(x)
# define Floor(x) M_Floor(x)

# define Radians(x) M_Radians((x))
# define Degrees(x) M_Degrees((x))
# define Sgn(x) M_Sgn((x))
#endif

__BEGIN_DECLS
void   M_InitSubsystem(void);
void   M_DestroySubsystem(void);

M_Real      M_ReadReal(AG_DataSource *);
void        M_CopyReal(AG_DataSource *, M_Real *);
void        M_WriteReal(AG_DataSource *, M_Real);
M_Complex   M_ReadComplex(AG_DataSource *);
void        M_CopyComplex(AG_DataSource *, M_Complex *);
void        M_WriteComplex(AG_DataSource *, M_Complex);
M_Range     M_ReadRange(AG_DataSource *);
void        M_CopyRange(AG_DataSource *, M_Range *);
void        M_WriteRange(AG_DataSource *, M_Range);
#define     M_ReadTime(ds) (M_Time)M_ReadReal(ds)
#define     M_WriteTime(ds,t) M_WriteReal((ds),(M_Real)(t))
M_TimeRange M_ReadTimeRange(AG_DataSource *);
void        M_CopyTimeRange(AG_DataSource *, M_TimeRange *);
void        M_WriteTimeRange(AG_DataSource *, M_TimeRange);

int    M_HeapSort(void *, size_t, size_t, int (*)(const void *, const void *));
void   M_QSort(void *, size_t, size_t, M_Real (*)(const void *, const void *));
int    M_MergeSort(void *, size_t, size_t, int (*)(const void *, const void *));
int    M_RadixSort(const Uint8 **, int, const Uint8 *, Uint);
int    M_RadixSortStable(const Uint8 **, int, const Uint8 *, Uint);
__END_DECLS
