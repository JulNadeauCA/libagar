/*	Public domain	*/

#undef _AGAR_BEGIN_H_

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

/* Undo begin.h definitions. */
#ifdef _AGAR_DEFINED_DECLSPEC
# undef _AGAR_DEFINED_DECLSPEC
# undef DECLSPEC
#endif
#ifdef _AGAR_DEFINED_NULL
# undef _AGAR_DEFINED_NULL
# undef NULL
#endif
