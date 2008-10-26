/*	Public domain	*/

#ifndef _AGAR_DEV_BEGIN_H_
#error Inclusion of <agar/dev/close.h> without <agar/dev/begin.h>
#endif
#undef _AGAR_DEV_BEGIN_H_

/*
 * Undo standard definitions from begin.h, unless we are compiling the
 * library itself.
 */
#ifndef _AGAR_DEV_INTERNAL
# ifdef _AGAR_DEV_DEFINED_UNSIGNED_TYPEDEFS
#  undef _AGAR_DEV_DEFINED_UNSIGNED_TYPEDEFS
#  undef Uint
#  undef Uchar
#  undef Ulong
# endif
# ifdef _AGAR_DEV_DEFINED_CDECLS
#  undef _AGAR_DEV_DEFINED_CDECLS
#  undef __BEGIN_DECLS
#  undef __END_DECLS
# endif
# ifdef _AGAR_DEV_DEFINED_NLS
#  undef _
#  undef N_
#  undef ngettext
# endif
#endif
#undef _AGAR_DEV_USE_INLINE

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
#ifdef _AGAR_DEV_DEFINED_DECLSPEC
# undef _AGAR_DEV_DEFINED_DECLSPEC
# undef DECLSPEC
#endif
#ifdef _AGAR_DEV_DEFINED_NULL
# undef _AGAR_DEV_DEFINED_NULL
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
