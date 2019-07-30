/*	Public domain	*/

#ifndef _AGAR_RG_BEGIN_H_
#error Inclusion of <agar/rg/close.h> without <agar/rg/begin.h>
#endif
#undef _AGAR_RG_BEGIN_H_

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

#undef _AGAR_RG_USE_INLINE

#ifndef _AGAR_RG_INTERNAL
/* Types */
# include <agar/core/close_types.h>
/* Declarations */
# ifdef _AGAR_RG_DEFINED_CDECLS
#  undef _AGAR_RG_DEFINED_CDECLS
#  undef __BEGIN_DECLS
#  undef __END_DECLS
# endif
# ifdef _AGAR_RG_DEFINED_DECLSPEC
#  undef _AGAR_RG_DEFINED_DECLSPEC
#  undef DECLSPEC
# endif
/* NLS */
# ifdef _AGAR_RG_DEFINED_NLS
#  undef _
#  undef N_
#  undef ngettext
# endif
/* Nullability */
# ifdef _AGAR_RG_DEFINED_NULL
#  undef _AGAR_RG_DEFINED_NULL
#  undef NULL
# endif
# if defined(_AGAR_RG_DEFINED_NULLABILITY) && !defined(_USE_AGAR_NULLABILITY)
#  undef _Nonnull
#  undef _Nullable
#  undef _Null_unspecified
# endif
#endif /* _AGAR_RG_INTERNAL */

#include <agar/core/close_attributes.h>
