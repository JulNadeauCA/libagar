/*	Public domain	*/

#ifndef _AGAR_CORE_ERROR_H_
#define _AGAR_CORE_ERROR_H_

#include <stdio.h>	/* XXX for fprintf, abort */
#include <stdlib.h>

#ifdef _AGAR_INTERNAL
#include <config/debug.h>
#else
#include <agar/config/debug.h>
#endif

#ifdef __GNUC__
#define fatal(fmt, args...)						\
	do {								\
		fprintf(stderr, "%s: " fmt, __FUNCTION__ , ##args);	\
		fprintf(stderr, "\n");					\
		abort();						\
	} while (0)
#else
#define fatal AG_FatalError
#endif

#define Malloc(len) AG_Malloc(len)
#define Free(p) AG_Free(p)
#define Realloc(p,len) AG_Realloc((p),(len))

#define Strdup(s) AG_Strdup(s)
#define Vasprintf(msg, fmt, args) do {				\
	va_start((args), (fmt));				\
	if (AG_Vasprintf((msg), (fmt), (args)) == -1) 		\
		fatal("vasprintf");				\
	va_end((args));						\
} while (0)

#ifdef _AGAR_INTERNAL
#include <core/error_pub.h>
#else
#include <agar/core/error_pub.h>
#endif

#define Verbose AG_Verbose

#ifdef DEBUG
#ifdef __GNUC__
# define dprintf(fmt,args...) printf("%s: " fmt, __FUNCTION__ , ##args)
# define debug(mask,fmt,args...) \
 if(agDebugLvl&(mask)) printf("%s: " fmt, __FUNCTION__ , ##args)
# define debug_n(mask,fmt,args...) \
 if(agDebugLvl&(mask)) fprintf(stderr, fmt, ##args)
# define Debug(obj,fmt,args...) \
 if(OBJECT_DEBUG(obj) || agDebugLvl > 0) \
   printf("%s: %s: " fmt, OBJECT(obj)->name, __FUNCTION__ , ##args)
#else
# define dprintf AG_DebugPrintf
# define deprintf AG_DebugPrintf
# define debug AG_Debug
# define debug_n AG_DebugN
# define Debug AG_DebugObj
#endif
#else
#if defined(__GNUC__)
# define dprintf(arg...) ((void)0)
# define deprintf(arg...) ((void)0)
# define debug(level, arg...) ((void)0)
# define debug_n(level, arg...) ((void)0)
# define Debug(obj, arg...) ((void)0)
#else
# define dprintf AG_DebugPrintf
# define deprintf AG_DebugPrintf
# define debug AG_Debug
# define debug_n AG_Debug
# define Debug AG_DebugObj
#endif
#endif

#include "begin_code.h"
#ifdef DEBUG
extern int agDebugLvl;
#endif
#include "close_code.h"

#undef FREE_NULL_IS_A_NOOP
#endif /* _AGAR_CORE_ERROR_H_ */
