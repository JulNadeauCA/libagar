/*	Public domain	*/

#ifndef _AGAR_AU_MATH_H_
#define _AGAR_AU_MATH_H_

#include <agar/config/have_math.h>
#include <agar/config/have_math_c99.h>
#ifdef HAVE_MATH
#include <math.h>
#endif

#include <agar/au/begin.h>

#ifdef M_PI
#define AU_PI M_PI
#else
#define	AU_PI 3.14159265358979323846
#endif

#ifdef HAVE_MATH_C99
# define AU_Sin(x) sinf(x)
# define AU_Cos(x) cosf(x)
# define AU_Tan(x) tanf(x)
# define AU_Mod(x,y) fmodf((x),(y))
# define AU_Sqrt(x) sqrtf(x)
# define AU_Atan2(y,x) atan2f((y),(x))
# define AU_Floor(x) floorf(x)
# define AU_Ceil(x) ceilf(x)
# define AU_Fabs(x) fabsf(x)
#else
# define AU_Sin(x) ((float)sin((double)x))
# define AU_Cos(x) ((float)cos((double)x))
# define AU_Tan(x) ((float)tan((double)x))
# define AU_Mod(x,y) ((float)fmod((double)(x),(double)(y)))
# define AU_Sqrt(x) ((float)sqrt((double)x))
# define AU_Atan2(y,x) ((float)atan2((double)(y),(double)(x)))
# define AU_Floor(x) ((float)floor((double)x))
# define AU_Ceil(x) ((float)ceil((double)x))
# define AU_Fabs(x) ((float)fabs((double)x))
#endif /* HAVE_MATH_C99 */

#define AU_Degrees(x) ((x)/(2.0*AU_PI)*360.0)
#define AU_Radians(x) (((x)/360.0)*(2.0*AU_PI))
#define AU_DotProd2(ax,ay,bx,by) ((ax)*(bx) + (ay)*(by))
#define AU_Norm2(ax,ay)	AU_Sqrt(AU_DotProd2((ax),(ay),(ax),(ay)))
#define AU_Distance2(ax,ay,bx,by) AU_Norm2((float)((ax)-(bx)),\
                                           (float)((ay)-(by)))

#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_AU_MATH)
#define Sin(x) AU_Sin(x)
#define Cos(x) AU_Cos(x)
#define Tan(x) AU_Tan(x)
#define Mod(x,y) AU_Mod((x),(y))
#define Sqrt(x) AU_Sqrt(x)
#define Atan2(y,x) AU_Atan2((y),(x))
#define Floor(x) AU_Floor(x)
#define Ceil(x) AU_Ceil(x)
#define Fabs(x) AU_Fabs(x)

#define Degrees(x) AU_Degrees(x)
#define Radians(x) AU_Radians(x)
#define DotProd2(ax,ay,bx,by) AU_DotProd2((ax),(ay),(bx),(by))
#define DotNorm2(ax,ay) AU_Norm2((ax),(ay))
#define Distance2(ax,ay,bx,by) AU_Distance2((ax),(ay),(bx),(by))
#define PowOf2i(x) AU_PowOf2i(x)
#define Hypot(x,y) AU_Hypot(x,y)
#define Truncf(x) AU_Truncf(x)
#define Fracf(x) AU_Fracf(x)
#define FracInvf(x) AU_FracInvf(x)
#endif /* _AGAR_INTERNAL or _USE_AGAR_AU_MATH */

__BEGIN_DECLS
static __inline__ int
AU_PowOf2i(int i)
{
	int val = 1;
	while (val < i) { val <<= 1; }
	return (val);
}
static __inline__ float AU_Hypot(float x, float y) { return AU_Sqrt(x*x+y*y); }
static __inline__ int AU_Truncf(double d) { return (int)floor(d); }
static __inline__ double AU_Fracf(double d) { return (d - floor(d)); }
static __inline__ double AU_FracInvf(double d) { return (1 - (d - floor(d))); }
__END_DECLS

#include <agar/au/close.h>
#endif /* _AGAR_AU_MATH_H_ */
