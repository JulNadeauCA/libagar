/*	Public domain	*/

#ifndef _AGAR_GUI_MATH_H_
#define _AGAR_GUI_MATH_H_

#include <agar/config/have_math.h>
#include <agar/config/have_math_c99.h>
#ifdef HAVE_MATH
#include <math.h>
#endif

#include <agar/gui/begin.h>

#ifdef M_PI
#define AG_PI M_PI
#else
#define	AG_PI 3.14159265358979323846
#endif

#ifdef HAVE_MATH_C99
# define AG_Sin(x) sinf(x)
# define AG_Cos(x) cosf(x)
# define AG_Tan(x) tanf(x)
# define AG_Mod(x,y) fmodf((x),(y))
# define AG_Sqrt(x) sqrtf(x)
# define AG_Atan2(y,x) atan2f((y),(x))
# define AG_Floor(x) floorf(x)
# define AG_Ceil(x) ceilf(x)
# define AG_Fabs(x) fabsf(x)
#else
# define AG_Sin(x) ((float)sin((double)x))
# define AG_Cos(x) ((float)cos((double)x))
# define AG_Tan(x) ((float)tan((double)x))
# define AG_Mod(x,y) ((float)fmod((double)(x),(double)(y)))
# define AG_Sqrt(x) ((float)sqrt((double)x))
# define AG_Atan2(y,x) ((float)atan2((double)(y),(double)(x)))
# define AG_Floor(x) ((float)floor((double)x))
# define AG_Ceil(x) ((float)ceil((double)x))
# define AG_Fabs(x) ((float)fabs((double)x))
#endif /* HAVE_MATH_C99 */

#define AG_Degrees(x) ((x)/(2.0*AG_PI)*360.0)
#define AG_Radians(x) (((x)/360.0)*(2.0*AG_PI))
#define AG_DotProd2(ax,ay,bx,by) ((ax)*(bx) + (ay)*(by))
#define AG_Norm2(ax,ay)	AG_Sqrt(AG_DotProd2((ax),(ay),(ax),(ay)))
#define AG_Distance2(ax,ay,bx,by) AG_Norm2((float)((ax)-(bx)),\
                                           (float)((ay)-(by)))

#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_GUI_MATH)
#define Sin(x) AG_Sin(x)
#define Cos(x) AG_Cos(x)
#define Tan(x) AG_Tan(x)
#define Mod(x,y) AG_Mod((x),(y))
#define Sqrt(x) AG_Sqrt(x)
#define Atan2(y,x) AG_Atan2((y),(x))
#define Floor(x) AG_Floor(x)
#define Ceil(x) AG_Ceil(x)
#define Fabs(x) AG_Fabs(x)

#define Degrees(x) AG_Degrees(x)
#define Radians(x) AG_Radians(x)
#define DotProd2(ax,ay,bx,by) AG_DotProd2((ax),(ay),(bx),(by))
#define DotNorm2(ax,ay) AG_Norm2((ax),(ay))
#define Distance2(ax,ay,bx,by) AG_Distance2((ax),(ay),(bx),(by))
#define PowOf2i(x) AG_PowOf2i(x)
#define Hypot(x,y) AG_Hypot(x,y)
#define Truncf(x) AG_Truncf(x)
#define Fracf(x) AG_Fracf(x)
#define FracInvf(x) AG_FracInvf(x)
#endif /* _AGAR_INTERNAL or _USE_AGAR_GUI_MATH */

__BEGIN_DECLS
static __inline__ int
AG_PowOf2i(int i)
{
	int val = 1;
	while (val < i) { val <<= 1; }
	return (val);
}
static __inline__ float
AG_Hypot(float x, float y)
{
	return AG_Sqrt(x*x+y*y);
}
static __inline__ int
AG_Truncf(double d)
{
	return ((int)floor(d));
}
static __inline__ double
AG_Fracf(double d)
{
	return (d - floor(d));
}
static __inline__ double
AG_FracInvf(double d)
{
	return (1 - (d - floor(d)));
}
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_MATH_H_ */
