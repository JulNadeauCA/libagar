/*	Public domain	*/

#ifndef _AGAR_CORE_BYTESWAP_H_
#define _AGAR_CORE_BYTESWAP_H_

#ifdef _AGAR_INTERNAL
# include <config/have_64bit.h>
# include <config/_mk_big_endian.h>
# include <config/_mk_little_endian.h>
#else
# include <agar/config/have_64bit.h>
# include <agar/config/_mk_big_endian.h>
# include <agar/config/_mk_little_endian.h>
#endif

#include "begin_code.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Swap 16-bit
 */
#if defined(__GNUC__) && defined(__i386__) && \
   !(__GNUC__ == 2 && __GNUC_MINOR__ == 95)
static __inline__ Uint16
AG_Swap16(Uint16 x)
{
	__asm__("xchgb %b0,%h0" :
	        "=q" (x) :
		"0" (x));
	return (x);
}
#elif defined(__GNUC__) && defined(__x86_64__)
static __inline__ Uint16
AG_Swap16(Uint16 x)
{
	__asm__("xchgb %b0,%h0" :
	        "=Q" (x) :
		"0" (x));
	return (x);
}
#elif defined(__GNUC__) && (defined(__powerpc__) || defined(__ppc__))
static __inline__ Uint16
AG_Swap16(Uint16 x)
{
	Uint16 rv;
	__asm__("rlwimi %0,%2,8,16,23" :
	        "=&r" (rv) :
		"0" (x >> 8), "r" (x));
	return (rv);
}
#else
static __inline__ Uint16
AG_Swap16(Uint16 x)
{
	return ((x<<8)|(x>>8));
}
#endif

/*
 * Swap 32-bit
 */
#if defined(__GNUC__) && defined(__i386__)
static __inline__ Uint32
AG_Swap32(Uint32 x)
{
	__asm__("bswap %0" :
	        "=r" (x) :
		"0" (x));
	return (x);
}
#elif defined(__GNUC__) && defined(__x86_64__)
static __inline__ Uint32
AG_Swap32(Uint32 x)
{
	__asm__("bswapl %0" :
	        "=r" (x) :
		"0" (x));
	return (x);
}
#elif defined(__GNUC__) && (defined(__powerpc__) || defined(__ppc__))
static __inline__ Uint32
AG_Swap32(Uint32 x)
{
	Uint32 rv;
	__asm__("rlwimi %0,%2,24,16,23" :
	        "=&r" (rv) :
		"0" (x>>24), "r" (x));
	__asm__("rlwimi %0,%2,8,8,15" :
	        "=&r" (rv) :
		"0" (rv), "r" (x));
	__asm__("rlwimi %0,%2,24,0,7" :
	        "=&r" (rv) :
		"0" (rv), "r" (x));
	return (rv);
}
#else
static __inline__ Uint32
AG_Swap32(Uint32 x)
{
	return ((x << 24) |
	       ((x << 8) & 0x00FF0000) | ((x >> 8) & 0x0000FF00) | 
	        (x >> 24));
}
#endif

/*
 * Swap 64-bit
 */
#ifdef HAVE_64BIT
#if defined(__GNUC__) && defined(__i386__)
static __inline__ Uint64
AG_Swap64(Uint64 x)
{
	union { 
		struct { Uint32 a, b; } s;
		Uint64 u;
	} v;
	v.u = x;
	__asm__("bswapl %0 ; bswapl %1 ; xchgl %0,%1" :
	        "=r" (v.s.a), "=r" (v.s.b) :
		"0" (v.s.a), "1" (v.s.b)); 
	return (v.u);
}
#elif defined(__GNUC__) && defined(__x86_64__)
static __inline__ Uint64
AG_Swap64(Uint64 x)
{
	__asm__("bswapq %0" :
	        "=r" (x) :
		"0" (x));
	return (x);
}
#else
static __inline__ Uint64
AG_Swap64(Uint64 x)
{
	Uint32 high, low;

	low = (Uint32)(x & 0xFFFFFFFF);
	x >>= 32;
	high = (Uint32)(x & 0xFFFFFFFF);
	x = AG_Swap32(low);
	x <<= 32;
	x |= AG_Swap32(high);
	return (x);
}
#endif
#endif /* HAVE_64BIT */

#if AG_BYTEORDER == AG_BIG_ENDIAN
# define AG_SwapLE16(X)	AG_Swap16(X)
# define AG_SwapLE32(X)	AG_Swap32(X)
# define AG_SwapLE64(X)	AG_Swap64(X)
# define AG_SwapBE16(X)	(X)
# define AG_SwapBE32(X)	(X)
# define AG_SwapBE64(X)	(X)
#else
# define AG_SwapLE16(X)	(X)
# define AG_SwapLE32(X)	(X)
# define AG_SwapLE64(X)	(X)
# define AG_SwapBE16(X)	AG_Swap16(X)
# define AG_SwapBE32(X)	AG_Swap32(X)
# define AG_SwapBE64(X)	AG_Swap64(X)
#endif

#ifdef __cplusplus
}
#endif
#include "close_code.h"

#endif /* _AGAR_CORE_BYTESWAP_H_ */
