/*	$Csoft: vg_math.h,v 1.4 2004/05/24 03:32:22 vedge Exp $	*/
/*	Public domain	*/

#ifdef SG_DOUBLE_PRECISION
typedef double SG_Real;
#define SG_REAL(n) AG_DOUBLE(n)
#else
typedef float SG_Real;
#define SG_REAL(n) AG_FLOAT(n)
#endif

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

#define SG_Radians(x) ((x)/360.0*2.0*M_PI)
#define SG_Degrees(x) (360.0*((x)/(2.0*M_PI)))

__BEGIN_DECLS
__inline__ SG_Real SG_Rad2Deg(SG_Real);
__inline__ SG_Real SG_Deg2Rad(SG_Real);

SG_Real SG_ReadReal(AG_Netbuf *);
void	SG_CopyReal(AG_Netbuf *, SG_Real *);
void	SG_WriteReal(AG_Netbuf *, SG_Real);
__END_DECLS
