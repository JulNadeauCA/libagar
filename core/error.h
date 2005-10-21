/*	$Csoft: error.h,v 1.11 2005/09/27 00:25:17 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_CORE_ERROR_H_
#define _AGAR_CORE_ERROR_H_

#include <stdio.h>	/* XXX for fprintf, abort */
#include <stdlib.h>

#include <agar/config/debug.h>
#include <agar/config/free_null_is_a_noop.h>

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

#define Malloc(len,t) AG_Malloc((len),(t))
#define Realloc(p,len) AG_Realloc((p),(len))

#ifdef DEBUG
#define Free(p,t) AG_Free((p),(t))
#else
# ifdef FREE_NULL_IS_A_NOOP
# define Free(p,t) free(p)
# else
# define Free(p,t) if ((p)!=NULL) free(p)
# endif
#endif

#define Strdup(s) AG_Strdup(s)
#define Vasprintf(msg, fmt, args) do {				\
	va_start((args), (fmt));				\
	if (AG_Vasprintf((msg), (fmt), (args)) == -1) 		\
		fatal("vasprintf");				\
	va_end((args));						\
} while (0)

#include <agar/core/error_pub.h>

#ifdef DEBUG
#ifdef __GNUC__
# define dprintf(fmt,args...) printf("%s: " fmt, __FUNCTION__ , ##args)
# define debug(mask,fmt,args...) \
 if(agDebugLvl&(mask)) printf("%s: " fmt, __FUNCTION__ , ##args)
# define debug_n(mask,fmt,args...) \
 if(agDebugLvl&(mask)) fprintf(stderr, fmt, ##args)
#else
# define dprintf	AG_DebugPrintf
# define deprintf	AG_DebugPrintf
# define debug		AG_Debug
# define debug_n	AG_DebugN
#endif
#else
#if defined(__GNUC__)
# define dprintf(arg...) ((void)0)
# define deprintf(arg...) ((void)0)
# define debug(level, arg...) ((void)0)
# define debug_n(level, arg...) ((void)0)
#else
# define dprintf AG_DebugPrintfNop
# define deprintf AG_DebugPrintfNop
# define debug AG_DebugNop
# define debug_n AG_DebugNop
#endif
#endif

#include "begin_code.h"
#ifdef DEBUG
extern int agDebugLvl;
#endif
#include "close_code.h"

#undef FREE_NULL_IS_A_NOOP
#endif /* _AGAR_CORE_ERROR_H_ */
