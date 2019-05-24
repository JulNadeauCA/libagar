/*	Public domain	*/
/*
 * Serialization of floating-point numbers.
 */
#include <agar/config/ag_serialization.h>
#ifdef AG_SERIALIZATION

#include <agar/core/core.h>

#ifdef AG_HAVE_FLOAT
/* Import inlinables */
# undef AG_INLINE_HEADER
# include <agar/core/inline_load_real.h>
#endif /* AG_HAVE_FLOAT */

#endif /* AG_SERIALIZATION */
