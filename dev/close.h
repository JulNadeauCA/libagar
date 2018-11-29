/*	Public domain	*/

#ifndef _AGAR_DEV_BEGIN_H_
#error Inclusion of <agar/dev/close.h> without <agar/dev/begin.h>
#endif
#undef _AGAR_DEV_BEGIN_H_

/* Reset structure packing at previous byte alignment. */
#if defined(_MSC_VER) || defined(__MWERKS__) || defined(__WATCOMC__) || \
    defined(__BORLANDC__)
# ifdef __BORLANDC__
#  pragma nopackwarning
# endif
# if (defined(__MWERKS__) && defined(__MACOS__))
#  pragma options align=reset
#  pragma enumsalwaysint reset
# else
#  pragma pack(pop)
# endif
#endif

#undef _AGAR_DEV_USE_INLINE

#ifndef _AGAR_DEV_INTERNAL
/* Primitive types */
# include <agar/core/close_types.h>
/* Declarations */
# ifdef _AGAR_DEV_DEFINED_CDECLS
#  undef _AGAR_DEV_DEFINED_CDECLS
#  undef __BEGIN_DECLS
#  undef __END_DECLS
# endif
# ifdef _AGAR_DEV_DEFINED_DECLSPEC
#  undef _AGAR_DEV_DEFINED_DECLSPEC
#  undef DECLSPEC
# endif
/* NLS */
# ifdef _AGAR_DEV_DEFINED_NLS
#  undef _
#  undef N_
#  undef ngettext
# endif
/* Nullability */
# ifdef _AGAR_DEV_DEFINED_NULL
#  undef _AGAR_DEV_DEFINED_NULL
#  undef NULL
# endif
# ifdef _AGAR_DEV_DEFINED_NULLABILITY
#  undef _Nonnull
#  undef _Nullable
#  undef _Null_unspecified
# endif
/* Restrict */
# ifdef _AGAR_DEV_DEFINED_RESTRICT
#  undef _Restrict
# endif
#endif /* _AGAR_DEV_INTERNAL */

#include <agar/core/close_attributes.h>
