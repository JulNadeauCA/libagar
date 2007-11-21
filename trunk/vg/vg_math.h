/*	$Csoft: vg_math.h,v 1.4 2004/05/24 03:32:22 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_VG_MATH_H_
#define _AGAR_VG_MATH_H_
#include "begin_code.h"

#define VG_DotProd2(ax,ay,bx,by) \
	((ax)*(bx) + (ay)*(by))
#define VG_Norm2(ax,ay) \
	VG_Sqrt(VG_DotProd2((ax),(ay),(ax),(ay)))
#define VG_Distance2(ax,ay,bx,by) \
	VG_Norm2((float)((ax)-(bx)),(float)((ay)-(by)))


#if __STDC_VERSION__ >= 199901L
#define VG_Sin(x) sinf(x)
#define VG_Cos(x) cosf(x)
#define VG_Tan(x) tanf(x)
#define VG_Mod(x,y) fmodf((x),(y))
#define VG_Sqrt(x) sqrtf(x)
#define VG_Atan2(y,x) atan2f((y),(x))
#else
#define VG_Sin(x) ((float)sin((double)x))
#define VG_Cos(x) ((float)cos((double)x))
#define VG_Tan(x) ((float)tan((double)x))
#define VG_Mod(x,y) ((float)fmod((double)(x),(double)(y)))
#define VG_Sqrt(x) ((float)sqrt((double)x))
#define VG_Atan2(y,x) ((float)atan2((double)(y),(double)(x)))
#endif

#define	VG_PI 3.14159265358979323846

__BEGIN_DECLS
static __inline__ float
VG_Rad2Deg(float theta)
{
    return (theta/(2.0*VG_PI)*360.0);
}
static __inline__ float
VG_Deg2Rad(float theta)
{
    return ((theta/360.0)*(2.0*VG_PI));
}
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_VG_MATH_H_ */
