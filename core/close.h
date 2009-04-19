/*	Public domain	*/

#ifndef _AGAR_CORE_BEGIN_H_
#error Inclusion of <agar/core/close.h> without <agar/core/begin.h>
#endif
#undef _AGAR_CORE_BEGIN_H_

/*
 * Undo standard definitions from begin.h, unless we are compiling the
 * library itself.
 */
#ifndef _AGAR_CORE_INTERNAL
# undef Uint
# undef Uchar
# undef Ulong
# undef Uint8
# undef Sint8
# undef Uint16
# undef Sint16
# undef Uint32
# undef Sint32
# undef Uint64
# undef Sint64
# ifdef _AGAR_CORE_DEFINED_CDECLS
#  undef _AGAR_CORE_DEFINED_CDECLS
#  undef __BEGIN_DECLS
#  undef __END_DECLS
# endif
# ifdef _AGAR_CORE_DEFINED_NLS
#  undef _
#  undef N_
#  undef ngettext
# endif
#endif /* _AGAR_CORE_INTERNAL */

#undef _AGAR_CORE_USE_INLINE

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
#ifdef _AGAR_CORE_DEFINED_DECLSPEC
# undef _AGAR_CORE_DEFINED_DECLSPEC
# undef DECLSPEC
#endif
#ifdef _AGAR_CORE_DEFINED_NULL
# undef _AGAR_CORE_DEFINED_NULL
# undef NULL
#endif

/* Undo all FOO_ATTRIBUTE() definitions. */
#undef HAVE_BOUNDED_ATTRIBUTE
#undef HAVE_FORMAT_ATTRIBUTE
#undef HAVE_NONNULL_ATTRIBUTE
#undef HAVE_PACKED_ATTRIBUTE
#undef HAVE_ALIGNED_ATTRIBUTE
#undef BOUNDED_ATTRIBUTE
#undef FORMAT_ATTRIBUTE
#undef NONNULL_ATTRIBUTE
#undef PACKED_ATTRIBUTE
#undef ALIGNED_ATTRIBUTE
