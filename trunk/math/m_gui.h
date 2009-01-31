/*	Public domain	*/

#ifndef _AGAR_MATH_M_GUI_H_
#define _AGAR_MATH_M_GUI_H_
#include <agar/math/begin.h>

#if defined(QUAD_PRECISION)
# define M_NumericalNewReal			AG_NumericalNewLongDbl
# define M_NumericalNewRealR			AG_NumericalNewLongDblR
# define M_NumericalGetReal			AG_NumericalGetLongDbl
# define M_NumericalNewRealPNZ(p,f,u,l,v)	AG_NumericalNewLongDblR((p),(f),(u),(l),(v),M_TINYVAL,M_INFINITY)
# define M_NumericalNewRealPNZF(p,f,u,l,v)	AG_NumericalNewLongDblR((p),(f),(u),(l),(v),M_TINYVAL,M_HUGEVAL)
# define M_NumericalNewRealP(p,f,u,l,v)		AG_NumericalNewLongDblR((p),(f),(u),(l),(v),0.0,M_INFINITY)
# define M_NumericalNewRealPF(p,f,u,l,v)	AG_NumericalNewLongDblR((p),(f),(u),(l),(v),0.0,M_HUGEVAL)
#elif defined(DOUBLE_PRECISION)
# define M_NumericalNewReal	 		AG_NumericalNewDbl
# define M_NumericalNewRealR	 		AG_NumericalNewDblR
# define M_NumericalGetReal	 		AG_NumericalGetDbl
# define M_NumericalNewRealPNZ(p,f,u,l,v)	AG_NumericalNewDblR((p),(f),(u),(l),(v),M_TINYVAL,M_INFINITY)
# define M_NumericalNewRealPNZF(p,f,u,l,v)	AG_NumericalNewDblR((p),(f),(u),(l),(v),M_TINYVAL,M_HUGEVAL)
# define M_NumericalNewRealP(p,f,u,l,v)		AG_NumericalNewDblR((p),(f),(u),(l),(v),0.0,M_INFINITY)
# define M_NumericalNewRealPF(p,f,u,l,v)	AG_NumericalNewDblR((p),(f),(u),(l),(v),0.0,M_HUGEVAL)
#elif defined(SINGLE_PRECISION)
# define M_NumericalNewReal			AG_NumericalNewFlt
# define M_NumericalNewRealR			AG_NumericalNewFltR
# define M_NumericalGetReal	 		AG_NumericalGetFlt
# define M_NumericalNewRealPNZ(p,f,u,l,v)	AG_NumericalNewFltR((p),(f),(u),(l),(v),M_TINYVAL,M_INFINITY)
# define M_NumericalNewRealPNZF(p,f,u,l,v)	AG_NumericalNewFltR((p),(f),(u),(l),(v),M_TINYVAL,M_HUGEVAL)
# define M_NumericalNewRealP(p,f,u,l,v)		AG_NumericalNewFltR((p),(f),(u),(l),(v),0.0,M_INFINITY)
# define M_NumericalNewRealPF(p,f,u,l,v)	AG_NumericalNewFltR((p),(f),(u),(l),(v),0.0,M_HUGEVAL)
#else
# error "Precision is not defined"
#endif

#define M_NumericalNewTime	 	 AG_NumericalNewSint32
#define M_NumericalNewTimeR	 	 AG_NumericalNewSint32R
#define M_NumericalNewTimeP(p,f,u,l,v)	 AG_NumericalNewSint32R((p),(f),(u),(l),(v),0,0x7fffffff-1)
#define M_NumericalNewTimePNZ(p,f,u,l,v) AG_NumericalNewSint32R((p),(f),(u),(l),(v),1,0x7fffffff-1)
#define M_NumericalGetTime	 	 AG_NumericalGetSint32

__BEGIN_DECLS
void	*M_EditVector3(void *, const char *, M_Vector3 *);
void	*M_EditVector4(void *, const char *, M_Vector4 *);
void	*M_EditMatrix44(void *, const char *, M_Matrix44 *);
void	*M_EditTranslate3(void *, const char *, M_Matrix44 *);
void	*M_EditTranslate4(void *, const char *, M_Matrix44 *);
void	*M_EditScale3(void *, const char *, M_Matrix44 *);
void	*M_EditScale4(void *, const char *, M_Matrix44 *);
__END_DECLS

#include <agar/math/close.h>

#include <agar/math/m_plotter.h>
#include <agar/math/m_matview.h>

#endif /* _AGAR_MATH_M_GUI_H_ */
