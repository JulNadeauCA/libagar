/*	Public domain	*/
/*
 * Compiler-specific attributes and annotations
 * (included by <core/begin.h>, <gui/begin.h>, etc.)
 */

#undef _Aligned_Attribute
#undef _Alloc_Align_Attribute
#undef _Alloc_Size_Attribute
#undef _Alloc_Size2_Attribute
#undef _Const_Attribute
#undef DEPRECATED_ATTRIBUTE
#undef FORMAT_ATTRIBUTE
#undef _Malloc_Like_Attribute
#undef _Noreturn_Attribute
#undef _Packed_Attribute
#undef _Pure_Attribute
#undef _Pure_Attribute_If_Unthreaded
#undef _Section_Attribute
#undef _Unused_Variable_Attribute
#undef _Used_Variable_Attribute
#undef _Warn_Unused_Result
#undef _Weak_Attribute

#include <agar/config/ag_use_attributes.h>
#if defined(lint) || !defined(AG_USE_ATTRIBUTES)
# define _Aligned_Attribute(x)
# define _Alloc_Align_Attribute(x)
# define _Alloc_Size_Attribute(x)
# define _Alloc_Size2_Attribute(n,x)
# define _Const_Attribute
# define DEPRECATED_ATTRIBUTE
# define FORMAT_ATTRIBUTE(t,a,b)
# define _Malloc_Like_Attribute
# define _Packed_Attribute
# define _Pure_Attribute
# define _Pure_Attribute_If_Unthreaded
# define _Section_Attribute(x)
# define _Unused_Variable_Attribute
# define _Used_Variable_Attribute
# define _Warn_Unused_Result
# define _Weak_Attribute
# define _Noreturn_Attribute

/* Absence of noreturn may break compilation so make an exception for it */
# ifdef __CC65__
#  define __GNUC_PREREQ__(x,y) 0
# endif
# if defined(__cplusplus) && __cplusplus >= 201103L
#  undef  _Noreturn_Attribute
#  define _Noreturn_Attribute [[noreturn]]
# else
#  if __GNUC__ == 2 && __GNUC_MINOR__ >= 5 && __GNUC_MINOR__ < 7 && !defined(__INTEL_COMPILER)
#   undef  _Noreturn_Attribute
#   define _Noreturn_Attribute __attribute__((__noreturn__))
#  endif
#  if __GNUC_PREREQ__(2, 7) || defined(__INTEL_COMPILER)
#   undef  _Noreturn_Attribute
#   define _Noreturn_Attribute __attribute__((__noreturn__))
#  endif
# endif

#else /* !lint and AG_USE_ATTRIBUTES */

# include <agar/config/have_aligned_attribute.h>
# include <agar/config/have_const_attribute.h>
# include <agar/config/have_deprecated_attribute.h>
# include <agar/config/have_format_attribute.h>
# include <agar/config/have_malloc_attribute.h>
# include <agar/config/have_packed_attribute.h>
# include <agar/config/have_pure_attribute.h>
# include <agar/config/have_warn_unused_result_attribute.h>
# include <agar/config/have_unused_variable_attribute.h>
# include <agar/config/have_noreturn_attribute.h>

# define _Weak_Attribute __attribute__((__weak__))

# undef _Aligned_Attribute
# ifdef HAVE_ALIGNED_ATTRIBUTE
#  define _Aligned_Attribute(x) __attribute__((__aligned__(x)))
# else
#  define _Aligned_Attribute(x)
# endif

# undef _Const_Attribute
# ifdef HAVE_CONST_ATTRIBUTE
#  define _Const_Attribute __attribute__((__const__))
# else
#  define _Const_Attribute
# endif

# undef DEPRECATED_ATTRIBUTE
# ifdef HAVE_DEPRECATED_ATTRIBUTE
#  define DEPRECATED_ATTRIBUTE __attribute__((__deprecated__))
# else
#  define DEPRECATED_ATTRIBUTE
# endif

# undef FORMAT_ATTRIBUTE
# ifdef HAVE_FORMAT_ATTRIBUTE
#  define FORMAT_ATTRIBUTE(t,a,b) __attribute__((__format__ (t,a,b)))
# else
#  define FORMAT_ATTRIBUTE(t,a,b)
# endif

# undef _Malloc_Like_Attribute
# ifdef HAVE_MALLOC_ATTRIBUTE
#  define _Malloc_Like_Attribute __attribute__((__malloc__))
# else
#  define _Malloc_Like_Attribute
# endif

# undef _Packed_Attribute
# ifdef HAVE_PACKED_ATTRIBUTE
#  define _Packed_Attribute __attribute__((__packed__))
# else
#  define _Packed_Attribute
# endif

# undef _Pure_Attribute
# ifdef HAVE_PURE_ATTRIBUTE
#  define _Pure_Attribute __attribute__((__pure__))
# else
#  define _Pure_Attribute
# endif

# ifdef AG_THREADS
#  define _Pure_Attribute_If_Unthreaded
# else
#  ifdef HAVE_PURE_ATTRIBUTE
#   define _Pure_Attribute_If_Unthreaded __attribute__((__pure__))
#  else
#   define _Pure_Attribute_If_Unthreaded
#  endif
# endif /* AG_THREADS */

# undef _Warn_Unused_Result
# ifdef HAVE_WARN_UNUSED_RESULT_ATTRIBUTE
#  define _Warn_Unused_Result __attribute__((__warn_unused_result__))
# else
#  define _Warn_Unused_Result
# endif

# undef _Unused_Variable_Attribute
# ifdef HAVE_UNUSED_VARIABLE_ATTRIBUTE
#  define _Unused_Variable_Attribute __attribute__((__unused__))
# else
#  define _Unused_Variable_Attribute
# endif

# undef _Noreturn_Attribute
# ifdef HAVE_NORETURN_ATTRIBUTE
#  define _Noreturn_Attribute __attribute__((__noreturn__))
# else
#  define _Noreturn_Attribute
# endif

#endif /* lint or !AG_USE_ATTRIBUTES */

#if defined(__CC65__) && !defined(__inline__)
# define __inline__
#endif
