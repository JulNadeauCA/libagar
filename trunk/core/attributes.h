/*	Public domain	*/

/*
 * Definitions of FOO_ATTRIBUTE() macros. These macros expand to compiler
 * attributes when they are available.
 */

#include <agar/config/have_aligned_attribute.h>
#include <agar/config/have_bounded_attribute.h>
#include <agar/config/have_const_attribute.h>
#include <agar/config/have_deprecated_attribute.h>
#include <agar/config/have_format_attribute.h>
#include <agar/config/have_nonnull_attribute.h>
#include <agar/config/have_noreturn_attribute.h>
#include <agar/config/have_packed_attribute.h>
#include <agar/config/have_pure_attribute.h>
#include <agar/config/have_warn_unused_result_attribute.h>
#include <agar/config/have_unused_variable_attribute.h>

#undef ALIGNED_ATTRIBUTE
#ifdef HAVE_ALIGNED_ATTRIBUTE
# define ALIGNED_ATTRIBUTE(a) __attribute__((__aligned__ (a)))
#else
# define ALIGNED_ATTRIBUTE(a)
#endif

#undef BOUNDED_ATTRIBUTE
#ifdef HAVE_BOUNDED_ATTRIBUTE
# define BOUNDED_ATTRIBUTE(t, a, b) __attribute__((__bounded__ (t,a,b)))
#else
# define BOUNDED_ATTRIBUTE(t, a, b)
#endif

#undef CONST_ATTRIBUTE
#ifdef HAVE_CONST_ATTRIBUTE
# define CONST_ATTRIBUTE __attribute__((__const__))
#else
# define CONST_ATTRIBUTE
#endif

#undef DEPRECATED_ATTRIBUTE
#ifdef HAVE_DEPRECATED_ATTRIBUTE
# define DEPRECATED_ATTRIBUTE __attribute__((__deprecated__))
#else
# define DEPRECATED_ATTRIBUTE
#endif

#undef FORMAT_ATTRIBUTE
#ifdef HAVE_FORMAT_ATTRIBUTE
# define FORMAT_ATTRIBUTE(t, a, b) __attribute__((__format__ (t,a,b)))
#else
# define FORMAT_ATTRIBUTE(t, a, b)
#endif

#undef NONNULL_ATTRIBUTE
#ifdef HAVE_NONNULL_ATTRIBUTE
# define NONNULL_ATTRIBUTE(a) __attribute__((__nonnull__ (a)))
#else
# define NONNULL_ATTRIBUTE(a)
#endif

#undef NORETURN_ATTRIBUTE
#ifdef HAVE_NORETURN_ATTRIBUTE
# define NORETURN_ATTRIBUTE __attribute__((__noreturn__))
#else
# define NORETURN_ATTRIBUTE
#endif

#undef PACKED_ATTRIBUTE
#ifdef HAVE_PACKED_ATTRIBUTE
# define PACKED_ATTRIBUTE __attribute__((__packed__))
#else
# define PACKED_ATTRIBUTE
#endif

#undef PURE_ATTRIBUTE
#ifdef HAVE_PURE_ATTRIBUTE
# define PURE_ATTRIBUTE __attribute__((__pure__))
#else
# define PURE_ATTRIBUTE
#endif

#undef WARN_UNUSED_RESULT_ATTRIBUTE
#ifdef HAVE_WARN_UNUSED_RESULT_ATTRIBUTE
# define WARN_UNUSED_RESULT_ATTRIBUTE __attribute__((__warn_unused_result__))
#else
# define WARN_UNUSED_RESULT_ATTRIBUTE
#endif

#undef UNUSED_VARIABLE_ATTRIBUTE
#ifdef HAVE_UNUSED_VARIABLE_ATTRIBUTE
# define UNUSED_VARIABLE_ATTRIBUTE __attribute__((unused))
#else
# define UNUSED_VARIABLE_ATTRIBUTE
#endif
