/*	Public domain	*/

/*
 * Definitions of FOO_ATTRIBUTE() macros. These macros expand to compiler
 * attributes when they are available.
 */

#include <agar/config/have_bounded_attribute.h>
#include <agar/config/have_format_attribute.h>
#include <agar/config/have_nonnull_attribute.h>
#include <agar/config/have_packed_attribute.h>
#include <agar/config/have_aligned_attribute.h>

#ifdef HAVE_BOUNDED_ATTRIBUTE
# define BOUNDED_ATTRIBUTE(t, a, b) __attribute__((__bounded__ (t,a,b)))
#else
# define BOUNDED_ATTRIBUTE(t, a, b)
#endif

#ifdef HAVE_FORMAT_ATTRIBUTE
# define FORMAT_ATTRIBUTE(t, a, b) __attribute__((__format__ (t,a,b)))
#else
# define FORMAT_ATTRIBUTE(t, a, b)
#endif

#ifdef HAVE_NONNULL_ATTRIBUTE
# define NONNULL_ATTRIBUTE(a) __attribute__((__nonnull__ (a)))
#else
# define NONNULL_ATTRIBUTE(a)
#endif

#ifdef HAVE_PACKED_ATTRIBUTE
# define PACKED_ATTRIBUTE __attribute__((__packed__))
#else
# define PACKED_ATTRIBUTE
#endif

#ifdef HAVE_ALIGNED_ATTRIBUTE
# define ALIGNED_ATTRIBUTE(a) __attribute__((__aligned__ (a)))
#else
# define ALIGNED_ATTRIBUTE(a)
#endif
