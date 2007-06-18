/*	Public domain	*/

#ifdef SG_DOUBLE_PRECISION
typedef double SG_Real;
#define SG_REAL(n) AG_DOUBLE(n)
#else
typedef float SG_Real;
#define SG_REAL(n) AG_FLOAT(n)
#endif

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

#ifdef SG_DOUBLE_PRECISION
#define SG_Sqrt(x) sqrt(x)
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
#define SG_Fabs(x) fabs(x)
#define SG_Pow(x,y) pow((x),(y))
#else
#define SG_Sqrt(r) sqrtf(r)
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
#define SG_Fabs(x) fabsf(x)
#define SG_Pow(x,y) powf((x),(y))
#endif

#define SG_Radians(x) ((x)/360.0*2.0*SG_PI)
#define SG_Degrees(x) (360.0*((x)/(2.0*SG_PI)))

__BEGIN_DECLS
__inline__ SG_Real SG_Rad2Deg(SG_Real);
__inline__ SG_Real SG_Deg2Rad(SG_Real);

SG_Real SG_ReadReal(AG_Netbuf *);
void	SG_CopyReal(AG_Netbuf *, SG_Real *);
void	SG_WriteReal(AG_Netbuf *, SG_Real);
__END_DECLS
