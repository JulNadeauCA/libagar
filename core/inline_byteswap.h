/*	Public domain	*/

/*
 * Byte swapping functions
 */

/*
 * Swap 16-bit
 */
#if defined(__GNUC__) && defined(__i386__) && \
   !(__GNUC__ == 2 && __GNUC_MINOR__ == 95)
# ifdef AG_INLINE_HEADER
static __inline__ Uint16
AG_Swap16(Uint16 x)
# else
Uint16
ag_swap16(Uint16 x)
# endif
{
	__asm__("xchgb %b0,%h0" :
	        "=q" (x) :
		"0" (x));
	return (x);
}
#elif (defined(__GNUC__) || defined(__clang__)) && defined(__x86_64__)
# ifdef AG_INLINE_HEADER
static __inline__ Uint16
AG_Swap16(Uint16 x)
# else
Uint16
ag_swap16(Uint16 x)
# endif
{
	__asm__("xchgb %b0,%h0" :
	        "=Q" (x) :
		"0" (x));
	return (x);
}
#elif (defined(__GNUC__) || defined(__clang__)) && (defined(__powerpc__) || defined(__ppc__))
# ifdef AG_INLINE_HEADER
static __inline__ Uint16
AG_Swap16(Uint16 x)
# else
Uint16
ag_swap16(Uint16 x)
# endif
{
	Uint16 rv;
	__asm__("rlwimi %0,%2,8,16,23" :
	        "=&r" (rv) :
		"0" (x >> 8), "r" (x));
	return (rv);
}
#else
# ifdef AG_INLINE_HEADER
static __inline__ Uint16 _Const_Attribute
AG_Swap16(Uint16 x)
# else
Uint16
ag_swap16(Uint16 x)
# endif
{
	return ((x << 8) | (x >> 8));
}
#endif

/*
 * Swap 32-bit
 */
#if (defined(__GNUC__) || defined(__clang__)) && defined(__i386__)
# ifdef AG_INLINE_HEADER
static __inline__ Uint32
AG_Swap32(Uint32 x)
# else
Uint32
ag_swap32(Uint32 x)
# endif
{
	__asm__("bswap %0" :
	        "=r" (x) :
		"0" (x));
	return (x);
}
#elif (defined(__GNUC__) || defined(__clang__)) && defined(__x86_64__)
# ifdef AG_INLINE_HEADER
static __inline__ Uint32
AG_Swap32(Uint32 x)
# else
Uint32
ag_swap32(Uint32 x)
# endif
{
	__asm__("bswapl %0" :
	        "=r" (x) :
		"0" (x));
	return (x);
}
#elif (defined(__GNUC__) || defined(__clang__)) && (defined(__powerpc__) || defined(__ppc__))
# ifdef AG_INLINE_HEADER
static __inline__ Uint32
AG_Swap32(Uint32 x)
# else
Uint32
ag_swap32(Uint32 x)
# endif
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
# ifdef AG_INLINE_HEADER
static __inline__ Uint32 _Const_Attribute
AG_Swap32(Uint32 x)
# else
Uint32
ag_swap32(Uint32 x)
# endif
{
	return ((x << 24) |
	       ((x << 8) & 0x00FF0000) | ((x >> 8) & 0x0000FF00) | 
	        (x >> 24));
}
#endif

/*
 * Swap 64-bit
 */
#ifdef AG_HAVE_64BIT

# if (defined(__GNUC__) || defined(__clang__)) && defined(__i386__)
# ifdef AG_INLINE_HEADER
static __inline__ Uint64
AG_Swap64(Uint64 x)
# else
Uint64
ag_swap64(Uint64 x)
# endif
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
# elif (defined(__GNUC__) || defined(__clang__)) && defined(__x86_64__)
#  ifdef AG_INLINE_HEADER
static __inline__ Uint64
AG_Swap64(Uint64 x)
#  else
Uint64
ag_swap64(Uint64 x)
#  endif
{
	__asm__("bswapq %0" :
	        "=r" (x) :
		"0" (x));
	return (x);
}
# else /* !((__GNUC__ || __clang__) && __x86_64__) */
#  ifdef AG_INLINE_HEADER
static __inline__ Uint64 _Const_Attribute
AG_Swap64(Uint64 x)
#  else
Uint64
ag_swap64(Uint64 x)
#  endif
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
# endif /* !((__GNUC__ || __clang__) && __x86_64__) */

#endif /* AG_HAVE_64BIT */
