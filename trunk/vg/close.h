/*	Public domain	*/

#ifndef _AGAR_VG_BEGIN_H_
#error Inclusion of <agar/vg/close.h> without <agar/vg/begin.h>
#endif
#undef _AGAR_VG_BEGIN_H_

/*
 * Undo standard definitions from begin.h, unless we are compiling the
 * library itself.
 */
#ifndef _AGAR_VG_INTERNAL
# include <agar/core/close_types.h>
# ifdef _AGAR_VG_DEFINED_CDECLS
#  undef _AGAR_VG_DEFINED_CDECLS
#  undef __BEGIN_DECLS
#  undef __END_DECLS
# endif
# ifdef _AGAR_VG_DEFINED_NLS
#  undef _
#  undef N_
#  undef ngettext
# endif
#endif /* _AGAR_VG_INTERNAL */

#undef _AGAR_VG_USE_INLINE

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

/* Always undo DECLSPEC and NULL. */
#ifdef _AGAR_VG_DEFINED_DECLSPEC
# undef _AGAR_VG_DEFINED_DECLSPEC
# undef DECLSPEC
#endif
#ifdef _AGAR_VG_DEFINED_NULL
# undef _AGAR_VG_DEFINED_NULL
# undef NULL
#endif

#include <agar/core/close_attributes.h>
