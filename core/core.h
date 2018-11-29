/*	Public domain	*/
/*
 * Main internal Agar-Core header file. External applications and libraries
 * should use <agar/core.h> instead.
 */

#ifdef _AGAR_INTERNAL
#ifndef _AGAR_CORE_CORE_H_
#define _AGAR_CORE_CORE_H_

/* Standard includes for inlines (XXX TODO remove) */
#include <agar/config/_mk_have_stdlib_h.h>
#ifdef _MK_HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <agar/config/_mk_have_unistd_h.h>
#ifdef _MK_HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>
#include <stdio.h>

/* Memory model, primitive integer and float types */
#include <agar/core/types.h>

/* Pull in commonly used options */
#include <agar/config/ag_debug.h>
#include <agar/config/ag_legacy.h>
#include <agar/config/ag_threads.h>
#include <agar/config/ag_type_safety.h>

/* Define byte order (AG_BYTEORDER = AG_{LITTLE,BIG}_ENDIAN) */
#define AG_BIG_ENDIAN 4321
#define AG_LITTLE_ENDIAN 1234
#include <agar/config/_mk_big_endian.h>
#include <agar/config/_mk_little_endian.h>
#if defined(_MK_BIG_ENDIAN)
# define AG_BYTEORDER AG_BIG_ENDIAN
#elif defined(_MK_LITTLE_ENDIAN)
# define AG_BYTEORDER AG_LITTLE_ENDIAN
#else
# error "Byte order is unknown"
#endif
#undef _MK_BIG_ENDIAN
#undef _MK_LITTLE_ENDIAN

/*
 * Library initialization routines including AG_InitCore().
 *
 * Note: First invocation of <core/begin.h> happens here, which defines
 * compiler attributes (until <core/close.h>).
 */
#include <agar/core/core_init.h>

/* Error handling and malloc wrapper */
#include <agar/core/error.h>

/* Linked list definitions */
#include <agar/core/queue.h>

/* AG_FOO_MIN and AG_FOO_MAX constants */
#include <agar/core/limits.h>

/* POSIX threads macros and wrappers */
#include <agar/core/threads.h>

/* String handling routines */
#include <agar/core/string.h>
#include <agar/core/snprintf.h>
#include <agar/core/vsnprintf.h>
#include <agar/core/vasprintf.h>
#include <agar/core/asprintf.h>

/* Internal MIN and MAX macros (note: inline code should use AG_{MIN,MAX}) */
#ifndef MIN
#define	MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define	MAX(a,b) (((a)>(b))?(a):(b))
#endif
#ifndef MIN3
#define	MIN3(a,b,c) MIN((a),MIN((b),(c)))
#endif
#ifndef MAX3
#define	MAX3(a,b,c) MAX((a),MAX((b),(c)))
#endif

#ifdef _MSC_VER
#pragma warning(disable: 4018)
#pragma warning(disable: 4267)
#pragma warning(disable: 4244)
#endif

/* Data source interface (serialization, files and streams) */
#include <agar/core/data_source.h>

/* AG_Swap*() endianness swap macros */
#include <agar/core/byteswap.h>

/* Serialization of primitive types */
#include <agar/core/load_integral.h>
#include <agar/core/load_real.h>
#include <agar/core/load_string.h>
#include <agar/core/load_version.h>

/* Agar-Core */
#include <agar/core/version.h>
#include <agar/core/object.h>
#include <agar/core/list.h>
#include <agar/core/tree.h>
#include <agar/core/tbl.h>
#include <agar/core/cpuinfo.h>
#include <agar/core/file.h>
#include <agar/core/dir.h>
#include <agar/core/dso.h>
#include <agar/core/db.h>
#include <agar/core/exec.h>
#include <agar/core/user.h>
#include <agar/core/net.h>

#endif /* !_AGAR_CORE_CORE_H_ */
#endif /* _AGAR_INTERNAL */
