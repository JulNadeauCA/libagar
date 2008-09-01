/*	Public domain	*/
/*
 * Clean up internal header definitions.
 */
#ifndef _M_INTERNAL
# ifdef _MK_HAVE_UNSIGNED_TYPEDEFS
#  undef _MK_HAVE_UNSIGNED_TYPEDEFS
#  undef Uint
#  undef Uchar
#  undef Ulong
# endif
# ifdef _M_DEFINED_CDECLS
#  undef _M_DEFINED_CDECLS
#  undef __BEGIN_DECLS
#  undef __END_DECLS
# endif

# undef SINGLE_PRECISION
# undef DOUBLE_PRECISION
# undef QUAD_PRECISION
# ifdef _M_UNDEFINED_SINGLE_PRECISION
#  define SINGLE_PRECISION
# endif
# ifdef _M_UNDEFINED_DOUBLE_PRECISION
#  define DOUBLE_PRECISION
# endif
# ifdef _M_UNDEFINED_QUAD_PRECISION
#  define QUAD_PRECISION
# endif
#endif /* !_M_INTERNAL */
