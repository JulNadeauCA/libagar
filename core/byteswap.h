/*	Public domain	*/

#ifndef _AGAR_CORE_BYTESWAP_H_
#define _AGAR_CORE_BYTESWAP_H_
#include <agar/core/begin.h>

__BEGIN_DECLS
/*
 * Inlinables
 */
Uint16 ag_swap16(Uint16);
Uint32 ag_swap32(Uint32);
#ifdef AG_HAVE_64BIT
Uint64 ag_swap64(Uint64);
#endif
#ifdef AG_INLINE_BYTESWAP
# define AG_INLINE_HEADER
# include <agar/core/inline_byteswap.h>
#else
# define AG_Swap16(x) ag_swap16(x)
# define AG_Swap32(x) ag_swap32(x)
# define AG_Swap64(x) ag_swap64(x)
#endif
__END_DECLS

#if AG_BYTEORDER == AG_BIG_ENDIAN
# define AG_SwapLE16(X)	AG_Swap16(X)
# define AG_SwapLE32(X)	AG_Swap32(X)
# define AG_SwapLE64(X)	AG_Swap64(X)
# define AG_SwapBE16(X)	(X)
# define AG_SwapBE32(X)	(X)
# define AG_SwapBE64(X)	(X)
# define AG_SwapBEFLT(X) (X)
# define AG_SwapBEDBL(X) (X)
# define AG_SwapBELDBL(X) (X)
#elif AG_BYTEORDER == AG_LITTLE_ENDIAN
# define AG_SwapLE16(X)	(X)
# define AG_SwapLE32(X)	(X)
# define AG_SwapLE64(X)	(X)
# define AG_SwapLEFLT(X) (X)
# define AG_SwapLEDBL(X) (X)
# define AG_SwapLELDBL(X) (X)
# define AG_SwapBE16(X)	AG_Swap16(X)
# define AG_SwapBE32(X)	AG_Swap32(X)
# define AG_SwapBE64(X)	AG_Swap64(X)
#else
# error "AG_BYTEORDER is undefined"
#endif

#include <agar/core/close.h>
#endif /* _AGAR_CORE_BYTESWAP_H_ */
