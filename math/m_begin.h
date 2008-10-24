/*	Public domain	*/
/*
 * Definitions internal to our headers
 */

#ifdef SINGLE_PRECISION
# define _M_UNDEFINED_SINGLE_PRECISION
# undef SINGLE_PRECISION
#endif
#ifdef DOUBLE_PRECISION
# define _M_UNDEFINED_DOUBLE_PRECISION
# undef DOUBLE_PRECISION
#endif
#ifdef QUAD_PRECISION
# define _M_UNDEFINED_QUAD_PRECISION
# undef QUAD_PRECISION
#endif
#include <agar/config/single_precision.h>
#include <agar/config/quad_precision.h>
#include <agar/config/double_precision.h>

#include <agar/begin.h>

#ifdef _AGAR_INTERNAL
# undef MAX
# define MAX(h,i) ((h) > (i) ? (h) : (i))
# undef MIN
# define MIN(l,o) ((l) < (o) ? (l) : (o))
#endif /* _AGAR_INTERNAL */
