/*	Public domain	*/

#include <agar/config/have_math.h>
#include <agar/config/have_math_c99.h>
#ifdef HAVE_MATH
#include <math.h>
#endif

#include <agar/map/begin.h>

#ifdef M_PI
#define MAP_PI M_PI
#else
#define	MAP_PI 3.14159265358979323846
#endif

#ifdef HAVE_MATH_C99
# define MAP_Sin(x) sinf(x)
# define MAP_Cos(x) cosf(x)
# define MAP_Tan(x) tanf(x)
# define MAP_Mod(x,y) fmodf((x),(y))
# define MAP_Sqrt(x) sqrtf(x)
# define MAP_Atan2(y,x) atan2f((y),(x))
# define MAP_Floor(x) floorf(x)
# define MAP_Ceil(x) ceilf(x)
# define MAP_Fabs(x) fabsf(x)
#else
# define MAP_Sin(x) ((float)sin((double)x))
# define MAP_Cos(x) ((float)cos((double)x))
# define MAP_Tan(x) ((float)tan((double)x))
# define MAP_Mod(x,y) ((float)fmod((double)(x),(double)(y)))
# define MAP_Sqrt(x) ((float)sqrt((double)x))
# define MAP_Atan2(y,x) ((float)atan2((double)(y),(double)(x)))
# define MAP_Floor(x) ((float)floor((double)x))
# define MAP_Ceil(x) ((float)ceil((double)x))
# define MAP_Fabs(x) ((float)fabs((double)x))
#endif /* HAVE_MATH_C99 */

#define MAP_Degrees(x) ((x)/(2.0*MAP_PI)*360.0)
#define MAP_Radians(x) (((x)/360.0)*(2.0*MAP_PI))
#define MAP_DotProd2(ax,ay,bx,by) ((ax)*(bx) + (ay)*(by))
#define MAP_Norm2(ax,ay) MAP_Sqrt(MAP_DotProd2((ax),(ay),(ax),(ay)))
#define MAP_Distance2(ax,ay,bx,by) MAP_Norm2((float)((ax)-(bx)),\
                                             (float)((ay)-(by)))

#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_MAP_MATH)
#define Sin(x) MAP_Sin(x)
#define Cos(x) MAP_Cos(x)
#define Tan(x) MAP_Tan(x)
#define Mod(x,y) MAP_Mod((x),(y))
#define Sqrt(x) MAP_Sqrt(x)
#define Atan2(y,x) MAP_Atan2((y),(x))
#define Floor(x) MAP_Floor(x)
#define Ceil(x) MAP_Ceil(x)
#define Fabs(x) MAP_Fabs(x)

#define Degrees(x) MAP_Degrees(x)
#define Radians(x) MAP_Radians(x)
#define DotProd2(ax,ay,bx,by) MAP_DotProd2((ax),(ay),(bx),(by))
#define DotNorm2(ax,ay) MAP_Norm2((ax),(ay))
#define Distance2(ax,ay,bx,by) MAP_Distance2((ax),(ay),(bx),(by))
#define PowOf2i(x) MAP_PowOf2i(x)
#endif /* _AGAR_INTERNAL or _USE_AGAR_MAP_MATH */

__BEGIN_DECLS
static __inline__ int
MAP_PowOf2i(int i)
{
	int val = 1;
	while (val < i) { val <<= 1; }
	return (val);
}
__END_DECLS

#include <agar/map/close.h>
