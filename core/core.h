/*	Public domain	*/
/*
 * Main internal Agar-Core header file. External applications and libraries
 * should use <agar/core.h> instead.
 */

#ifdef _AGAR_INTERNAL
#ifndef _AGAR_CORE_CORE_H_
#define _AGAR_CORE_CORE_H_

/* For [SU]intN types */
#include <core/types.h>

#include <config/ag_debug.h>
#include <config/ag_legacy.h>
#include <config/ag_threads.h>
#include <config/ag_network.h>

/* For threads types and use in inlines. */
#include <core/threads.h>

/* For inline routines */
#include <config/_mk_have_stdlib_h.h>
#ifdef _MK_HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <config/_mk_have_unistd_h.h>
#ifdef _MK_HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>
#include <stdio.h>

/* Define AG_BYTEORDER */
#define AG_BIG_ENDIAN 4321
#define AG_LITTLE_ENDIAN 1234
#include <config/_mk_big_endian.h>
#include <config/_mk_little_endian.h>
#if defined(_MK_BIG_ENDIAN)
# define AG_BYTEORDER AG_BIG_ENDIAN
#elif defined(_MK_LITTLE_ENDIAN)
# define AG_BYTEORDER AG_LITTLE_ENDIAN
#else
# error "Byte order is unknown"
#endif
#undef _MK_BIG_ENDIAN
#undef _MK_LITTLE_ENDIAN

#include <core/core_init.h>
#include <core/error.h>
#include <core/queue.h>
#include <core/limits.h>

#include <core/string_compat.h>
#include <core/snprintf.h>
#include <core/vsnprintf.h>
#include <core/vasprintf.h>

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

#include <core/data_source.h>
#include <core/load_integral.h>
#include <core/load_real.h>
#include <core/load_string.h>
#include <core/load_version.h>

#include <core/version.h>
#include <core/object.h>
#include <core/list.h>
#include <core/tree.h>
#include <core/tbl.h>
#include <core/cpuinfo.h>
#include <core/file.h>
#include <core/dir.h>
#include <core/dso.h>
#include <core/time.h>
#include <core/db.h>
#include <core/dbobject.h>
#include <core/exec.h>

#endif /* !_AGAR_CORE_CORE_H_ */
#endif /* _AGAR_INTERNAL */
