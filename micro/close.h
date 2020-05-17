/*	Public domain	*/

#ifndef _AGAR_MICRO_BEGIN_H_
#error Inclusion of <agar/micro/close.h> without <agar/micro/begin.h>
#endif
#undef _AGAR_MICRO_BEGIN_H_

/* Unwind Inlining */
#undef _AGAR_MICRO_USE_INLINE

#ifndef _AGAR_MICRO_INTERNAL
/* Unwind Primitive types */
# include <agar/core/close_types.h>
/* Unwind Declarations */
# ifdef _AGAR_MICRO_DEFINED_CDECLS
#  undef _AGAR_MICRO_DEFINED_CDECLS
#  undef __BEGIN_DECLS
#  undef __END_DECLS
# endif
# ifdef _AGAR_MICRO_DEFINED_DECLSPEC
#  undef _AGAR_MICRO_DEFINED_DECLSPEC
#  undef DECLSPEC
# endif
/* Unwind Nullability */
# ifdef _AGAR_MICRO_DEFINED_NULL
#  undef _AGAR_MICRO_DEFINED_NULL
#  undef NULL
# endif
# if defined(_AGAR_MICRO_DEFINED_NULLABILITY) && !defined(_USE_AGAR_NULLABILITY)
#  undef _Nonnull
#  undef _Nullable
#  undef _Null_unspecified
# endif
/* Unwind Restrict */
# ifdef _AGAR_MICRO_DEFINED_RESTRICT
#  undef _Restrict
# endif
#endif /* !_AGAR_MICRO_INTERNAL */

/* Unwind Compiler-specific attributes and annotations */
#include <agar/core/close_attributes.h>

