/*	$Csoft: view.h,v 1.107 2005/10/03 17:37:30 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_CORE_MATH_H_
#define _AGAR_CORE_MATH_H_
#include "begin_code.h"

#if 0
typedef Uint32 AG_Fixpt6;
typedef Uint32 AG_Fixpt8;
typedef Uint32 AG_Fixpt16;
typedef Uint32 AG_Fixpt24;
typedef Uint32 AG_Fixpt30;

#define AGFIX6_MAX_INT		(1<<25)
#define AGFIX8_MAX_INT		(1<<24)
#define AGFIX10_MAX_INT		(1<<22)
#define AGFIX16_MAX_INT		(1<<16)
#define AGFIX30_MAX_INT		(1<<2)

#define AGFIX6_MAX_FRAC		(1<<6)
#define AGFIX8_MAX_FRAC		(1<<8)
#define AGFIX10_MAX_FRAC	(1<<10)
#define AGFIX16_MAX_FRAC	(1<<16)
#define AGFIX30_MAX_FRAC	(1<<30)

#define AG_IntToFix6(i) ((i) << 6)
#define AG_IntToFix8(i) ((i) << 8)
#define AG_IntToFix10(i) ((i) << 10)
#define AG_IntToFix16(i) ((i) << 16)
#define AG_IntToFix30(i) ((i) << 30)

#define AG_Fix6ToInt(f) ((f) >> 6)
#define AG_Fix8ToInt(f) ((f) >> 8)
#define AG_Fix10ToInt(f) ((f) >> 10)
#define AG_Fix16ToInt(f) ((f) >> 16)
#define AG_Fix30ToInt(f) ((f) >> 30)

#define AG_Fix6Frac(f) ((f) << 26)
#define AG_Fix8Frac(f) ((f) << 24)
#define AG_Fix10Frac(f) ((f) << 22)
#define AG_Fix16Frac(f) ((f) << 16)
#define AG_Fix30Frac(f) ((f) << 2)

#define AG_Fix6InvFrac(f) (1-((f) << 26))
#define AG_Fix8InvFrac(f) (1-((f) << 24))
#define AG_Fix10InvFrac(f) (1-((f) << 22))
#define AG_Fix16InvFrac(f) (1-((f) << 16))
#define AG_Fix30InvFrac(f) (1-((f) << 2))

#define AG_FpToFix6(F) ((AG_Fixpt6)((F) * 64))
#define AG_FpToFix8(F) ((AG_Fixpt6)((F) * 256))
#define AG_FpToFix10(F) ((AG_Fixpt10)((F) * 1024))
#define AG_FpToFix16(F) ((AG_Fixpt16)((F) * 65536))
#define AG_FpToFix30(F) ((AG_Fixpt30)((F) * 2147483648))

#define AG_Fix6ToFp(f) (((float)(f)) / 64)
#define AG_Fix8ToFp(f) (((float)(f)) / 256)
#define AG_Fix10ToFp(f) (((float)(f)) / 1024)
#define AG_Fix16ToFp(f) (((float)(f)) / 65536)
#define AG_Fix30ToFp(f) (((float)(f)) / 2147483648)

#define AG_Fix6Mul(f1, f2) (((f1)*(f2)) >> 6)
#define AG_Fix8Mul(f1, f2) (((f1)*(f2)) >> 8)
#define AG_Fix10Mul(f1, f2) (((f1)*(f2)) >> 10)
#define AG_Fix16Mul(f1, f2) (((f1)*(f2)) >> 16)
#define AG_Fix30Mul(f1, f2) (((f1)*(f2)) >> 30)

#define AG_Fix6Div(f1, f2) (((f1) << 6) / (f2))
#define AG_Fix8Div(f1, f2) (((f1) << 8) / (f2))
#define AG_Fix10Div(f1, f2) (((f1) << 10) / (f2))
#define AG_Fix16Div(f1, f2) (((f1) << 16) / (f2))
#define AG_Fix30Div(f1, f2) (((f1) << 30) / (f2))
#endif

__BEGIN_DECLS
__inline__ int AG_PowOf2i(int);
__inline__ int AG_Truncf(double);
__inline__ double AG_Fracf(double);
__inline__ double AG_FracInvf(double);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_CORE_MATH_H_ */
