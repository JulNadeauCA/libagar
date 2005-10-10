/*	$Csoft: vg_math.h,v 1.4 2004/05/24 03:32:22 vedge Exp $	*/
/*	Public domain	*/

#ifdef SG_DOUBLE_PRECISION
typedef double SG_Real;
#else
typedef float SG_Real;
#endif

#ifdef SG_DOUBLE_PRECISION
#define AG_Sqrt(r) sqrt(r)
#else
#define AG_Sqrt(r) sqrtf(r)
#endif

typedef struct { SG_Real x, y; } SG_Complex;

__BEGIN_DECLS
__inline__ SG_Real SG_Rad2Deg(SG_Real);
__inline__ SG_Real SG_Deg2Rad(SG_Real);
__END_DECLS
