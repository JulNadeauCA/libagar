/*	Public domain	*/

#ifndef _AGAR_CORE_UTIL_H_
#define _AGAR_CORE_UTIL_H_
#include "begin_code.h"

__BEGIN_DECLS
static __inline__ int
AG_PowOf2i(int i)
{
	int val = 1;

	while (val < i) { val <<= 1; }
	return (val);
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

#include "close_code.h"
#endif /* _AGAR_CORE_UTIL_H_ */
