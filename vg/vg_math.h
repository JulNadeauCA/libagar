/*	Public domain	*/

#ifndef _AGAR_VG_MATH_H_
#define _AGAR_VG_MATH_H_

#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_VG_MATH)
# ifdef _AGAR_INTERNAL
#  include <config/have_math.h>
# else
#  include <agar/config/have_math.h>
# endif
# ifdef HAVE_MATH
#  include <math.h>
# endif
#endif

#include "begin_code.h"

#ifdef M_PI
# define VG_PI M_PI
#else
# define VG_PI 3.14159265358979323846
#endif

#if __STDC_VERSION__ >= 199901L
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
# define VG_Hypot(x,y) ((float)hypotf((double)(x),(double)(y)))
#endif /* C99 */

#define VG_Degrees(x) ((x)/(2.0*VG_PI)*360.0)
#define VG_Radians(x) (((x)/360.0)*(2.0*VG_PI))
#define VG_DotProd2(ax,ay,bx,by) ((ax)*(bx) + (ay)*(by))
#define VG_Norm2(ax,ay) \
	VG_Sqrt(VG_DotProd2((ax),(ay),(ax),(ay)))
#define VG_Distance2(ax,ay,bx,by) \
	VG_Norm2((float)((ax)-(bx)),(float)((ay)-(by)))

#define VG_Truncf(d) ((int)floor(d))
#define VG_Fracf(d) ((d) - floor(d))
#define VG_FracInvf(d) (1 - ((d) - floor(d)))

#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_VG_MATH)
#define Sin(x) VG_Sin(x)
#define Cos(x) VG_Cos(x)
#define Tan(x) VG_Tan(x)
#define Mod(x,y) VG_Mod((x),(y))
#define Sqrt(x) VG_Sqrt(x)
#define Atan2(y,x) VG_Atan2((y),(x))
#define Floor(x) VG_Floor(x)
#define Ceil(x) VG_Ceil(x)
#define Fabs(x) VG_Fabs(x)
#define Hypot(x) VG_Hypot(x)

#define Degrees(x) VG_Degrees(x)
#define Radians(x) VG_Radians(x)
#define DotProd2(ax,ay,bx,by) VG_DotProd2((ax),(ay),(bx),(by))
#define DotNorm2(ax,ay) VG_Norm2((ax),(ay))
#define Distance2(ax,ay,bx,by) VG_Distance2((ax),(ay),(bx),(by))

#define PowOf2i(x) VG_PowOf2i(x)
#define Truncf(x) VG_Truncf(x)
#define Fracf(x) VG_Fracf(x)
#define FracInvf(x) VG_FracInvf(x)
#endif /* _AGAR_INTERNAL or _USE_AGAR_VG_MATH */

__BEGIN_DECLS
extern int vg_cos_tbl[];
extern int vg_sin_tbl[];

static __inline__ int
VG_PowOf2i(int i)
{
	int val = 1;
	while (val < i) { val <<= 1; }
	return (val);
}
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_MATH_H_ */
