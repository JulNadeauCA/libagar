/*	Public domain	*/
/*
 * Clean up internal header definitions.
 */
#ifndef _AGAR_INTERNAL
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
#endif /* !_AGAR_INTERNAL */

#include <agar/close.h>
