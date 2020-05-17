/*	Public domain	*/

#ifndef _AGAR_MATH_M_GUI_H_
#define _AGAR_MATH_M_GUI_H_
#include <agar/math/begin.h>

#if defined(DOUBLE_PRECISION)
# define M_BindReal				AG_BindDouble
# define M_BindRealMp				AG_BindDoubleMp
# define M_BindRealFn				AG_BindDoubleFn
# define M_NumericalNewReal	 		AG_NumericalNewDbl
# define M_NumericalNewRealR	 		AG_NumericalNewDblR
# define M_NumericalGetReal	 		AG_NumericalGetDbl
# define M_NumericalNewRealPNZ(p,f,u,l,v)	AG_NumericalNewDblR((p),(f),(u),(l),(v),M_TINYVAL,M_INFINITY)
# define M_NumericalNewRealPNZF(p,f,u,l,v)	AG_NumericalNewDblR((p),(f),(u),(l),(v),M_TINYVAL,M_HUGEVAL)
# define M_NumericalNewRealP(p,f,u,l,v)		AG_NumericalNewDblR((p),(f),(u),(l),(v),0.0,M_INFINITY)
# define M_NumericalNewRealPF(p,f,u,l,v)	AG_NumericalNewDblR((p),(f),(u),(l),(v),0.0,M_HUGEVAL)
#elif defined(SINGLE_PRECISION)
# define M_BindReal				AG_BindFloat
# define M_BindRealMp				AG_BindFloatMp
# define M_BindRealFn				AG_BindFloatFn
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
void *_Nonnull M_EditTranslate3(void *_Nullable, const char *_Nullable,
                                M_Matrix44 *_Nonnull);
#ifdef AG_THREADS
void *_Nonnull M_EditTranslate3Mp(void *_Nullable, const char *_Nullable,
                                  M_Matrix44 *_Nonnull,
				  _Nonnull_Mutex AG_Mutex *_Nonnull);
#endif
__END_DECLS

#include <agar/math/close.h>

#include <agar/math/m_plotter.h>
#include <agar/math/m_matview.h>
#include <agar/math/m_bezier_primitives.h>

#endif /* _AGAR_MATH_M_GUI_H_ */
