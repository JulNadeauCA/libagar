/*	Public domain	*/

#ifndef _AGAR_RG_MATH_H_
#define _AGAR_RG_MATH_H_

#include <agar/config/have_math.h>
#include <agar/config/have_math_c99.h>
#ifdef HAVE_MATH
#include <math.h>
#endif

#include <agar/map/begin.h>

#ifdef M_PI
#define RG_PI M_PI
#else
#define	RG_PI 3.14159265358979323846
#endif

#ifdef HAVE_MATH_C99
# define RG_Sin(x) sinf(x)
# define RG_Cos(x) cosf(x)
# define RG_Tan(x) tanf(x)
# define RG_Mod(x,y) fmodf((x),(y))
# define RG_Sqrt(x) sqrtf(x)
# define RG_Atan2(y,x) atan2f((y),(x))
# define RG_Floor(x) floorf(x)
# define RG_Ceil(x) ceilf(x)
# define RG_Fabs(x) fabsf(x)
#else
# define RG_Sin(x) ((float)sin((double)x))
# define RG_Cos(x) ((float)cos((double)x))
# define RG_Tan(x) ((float)tan((double)x))
# define RG_Mod(x,y) ((float)fmod((double)(x),(double)(y)))
# define RG_Sqrt(x) ((float)sqrt((double)x))
# define RG_Atan2(y,x) ((float)atan2((double)(y),(double)(x)))
# define RG_Floor(x) ((float)floor((double)x))
# define RG_Ceil(x) ((float)ceil((double)x))
# define RG_Fabs(x) ((float)fabs((double)x))
#endif /* HAVE_MATH_C99 */

#define RG_Degrees(x) ((x)/(2.0*RG_PI)*360.0)
#define RG_Radians(x) (((x)/360.0)*(2.0*RG_PI))
#define RG_DotProd2(ax,ay,bx,by) ((ax)*(bx) + (ay)*(by))
#define RG_Norm2(ax,ay)	RG_Sqrt(RG_DotProd2((ax),(ay),(ax),(ay)))
#define RG_Distance2(ax,ay,bx,by) RG_Norm2((float)((ax)-(bx)),\
                                           (float)((ay)-(by)))

#if defined(_AGAR_RG_INTERNAL) || defined(_USE_AGAR_RG_MATH)
#define Sin(x) RG_Sin(x)
#define Cos(x) RG_Cos(x)
#define Tan(x) RG_Tan(x)
#define Mod(x,y) RG_Mod((x),(y))
#define Sqrt(x) RG_Sqrt(x)
#define Atan2(y,x) RG_Atan2((y),(x))
#define Floor(x) RG_Floor(x)
#define Ceil(x) RG_Ceil(x)
#define Fabs(x) RG_Fabs(x)

#define Degrees(x) RG_Degrees(x)
#define Radians(x) RG_Radians(x)
#define DotProd2(ax,ay,bx,by) RG_DotProd2((ax),(ay),(bx),(by))
#define DotNorm2(ax,ay) RG_Norm2((ax),(ay))
#define Distance2(ax,ay,bx,by) RG_Distance2((ax),(ay),(bx),(by))
#define PowOf2i(x) RG_PowOf2i(x)
#define Hypot(x,y) RG_Hypot(x,y)
#define Truncf(x) RG_Truncf(x)
#define Fracf(x) RG_Fracf(x)
#define FracInvf(x) RG_FracInvf(x)
#endif /* _AGAR_RG_INTERNAL or _USE_AGAR_RG_MATH */

__BEGIN_DECLS
static __inline__ int
RG_PowOf2i(int i)
{
	int val = 1;
	while (val < i) { val <<= 1; }
	return (val);
}
static __inline__ float RG_Hypot(float x, float y) { return RG_Sqrt(x*x+y*y); }
static __inline__ int RG_Truncf(double d) { return (int)floor(d); }
static __inline__ double RG_Fracf(double d) { return (d - floor(d)); }
static __inline__ double RG_FracInvf(double d) { return (1 - (d - floor(d))); }
__END_DECLS

#include <agar/map/close.h>
#endif /* _AGAR_RG_MATH_H_ */
