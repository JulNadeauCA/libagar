/*	Public domain	*/

#ifndef _AGAR_CORE_ERROR_H_
#define _AGAR_CORE_ERROR_H_

#ifdef _AGAR_INTERNAL
#include <config/debug.h>
#endif

#define Malloc(len) AG_Malloc(len)
#define Free(p) AG_Free(p)
#define Realloc(p,len) AG_Realloc((p),(len))
#define Strdup(s) AG_Strdup(s)
#define Snprintf AG_Snprintf
#define Vsnprintf AG_Vsnprintf
#define Vasprintf(msg, fmt, args) do {				\
	if (AG_Vasprintf((msg),(fmt),(args)) == -1) 		\
		AG_FatalError("Out of memory (vasprintf)");	\
} while (0)

#ifdef _AGAR_INTERNAL
#include <core/error_pub.h>
#else
#include <agar/core/error_pub.h>
#endif

#define Verbose AG_Verbose

#ifdef DEBUG
# define Debug AG_Debug
#else
# ifdef __GNUC__
#  define Debug(obj, arg...) ((void)0)
# else
#  define Debug AG_Debug
# endif
#endif /* DEBUG */

#include "begin_code.h"
#ifdef DEBUG
extern int agDebugLvl;
#endif
#include "close_code.h"

#undef FREE_NULL_IS_A_NOOP
#endif /* _AGAR_CORE_ERROR_H_ */
