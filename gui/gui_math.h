/*	Public domain	*/

#ifndef _AGAR_GUI_MATH_H_
#define _AGAR_GUI_MATH_H_

#include <agar/config/have_math.h>
#ifdef HAVE_MATH
#include <math.h>
#endif

#include <agar/gui/begin.h>

#ifdef M_PI
# define AG_PI M_PI
#else
# define AG_PI 3.14159265358979323846
#endif

#define AG_Sin(x)     sin(x)
#define AG_Cos(x)     cos(x)
#define AG_Tan(x)     tan(x)
#define AG_Mod(x,y)   fmod((x),(y))
#define AG_Sqrt(x)    sqrt(x)
#define AG_Atan2(y,x) atan2((y),(x))
#define AG_Floor(x)   floor(x)
#define AG_Ceil(x)    ceil(x)
#define AG_Fabs(x)    fabs(x)

#define AG_Degrees(x)            ((x)/(2.0*AG_PI)*360.0)
#define AG_Radians(x)            (((x)/360.0)*(2.0*AG_PI))
#define AG_DotProd2(ax,ay,bx,by) ((ax)*(bx) + (ay)*(by))
#define AG_Norm2(ax,ay)           AG_Sqrt(AG_DotProd2((ax),(ay),(ax),(ay)))
#define AG_Distance2(ax,ay,bx,by) AG_Norm2(((ax)-(bx)), ((ay)-(by)))

#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_GUI_MATH)
# define Sin(x)     AG_Sin(x)
# define Cos(x)     AG_Cos(x)
# define Tan(x)     AG_Tan(x)
# define Mod(x,y)   AG_Mod((x),(y))
# define Sqrt(x)    AG_Sqrt(x)
# define Atan2(y,x) AG_Atan2((y),(x))
# define Floor(x)   AG_Floor(x)
# define Ceil(x)    AG_Ceil(x)
# define Fabs(x)    AG_Fabs(x)

# define Degrees(x)         AG_Degrees(x)
# define Radians(x)         AG_Radians(x)
# define DotProd2(a,b,c,d)  AG_DotProd2((a),(b),(c),(d))
# define DotNorm2(ax,ay)    AG_Norm2((ax),(ay))
# define Distance2(a,b,c,d) AG_Distance2((a),(b),(c),(d))
# define Hypot(x,y)         AG_Hypot(x,y)
# define Truncf(x)          AG_Truncf(x)
# define Fracf(x)           AG_Fracf(x)
# define FracInvf(x)        AG_FracInvf(x)
# define Square(x)          AG_Square(x)
# define Distance(x1,y1,x2,y2) AG_Distance(x1, y1, x2, y2)

#endif /* _AGAR_INTERNAL or _USE_AGAR_GUI_MATH */

__BEGIN_DECLS
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
static __inline__ double
AG_Square(double d) {
	return d*d;
}
static __inline__ double
AG_Distance(double x1, double y1, double x2, double y2) {
	return AG_Sqrt(AG_Square(x2-x1) + AG_Square(y2-y1));
}
static __inline__ float
AG_Hypot(float x, float y)
{
	return AG_Sqrt(AG_Square(x)+AG_Square(y));
}
static __inline__ unsigned char
AG_HaveQuadraticSolution(double a, double b, double c) {
	return (((AG_Square(b) - 4 * a * c) >= 0) && a != 0);
}
static __inline__ double
AG_QuadraticPositive(double a, double b, double c) {
	return ((-1 * b) + AG_Sqrt(AG_Square(b) - 4 * a * c)) / ( 2 * a);
}
static __inline__ double
AG_QuadraticNegative(double a, double b, double c) {
	return ((-1 * b) - AG_Sqrt(AG_Square(b) - 4 * a * c)) / ( 2 * a);
}
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_MATH_H_ */
