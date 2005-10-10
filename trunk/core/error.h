/*	$Csoft: error.h,v 1.11 2005/09/27 00:25:17 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_CORE_ERROR_H_
#define _AGAR_CORE_ERROR_H_

#include <agar/compat/queue.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <agar/config/debug.h>
#include <agar/config/threads.h>
#include <agar/config/have_bounded_attribute.h>
#include <agar/config/have_format_attribute.h>
#include <agar/config/have_nonnull_attribute.h>

#if !defined(__BEGIN_DECLS) || !defined(__END_DECLS)
# if defined(__cplusplus)
#  define __BEGIN_DECLS	extern "C" {
#  define __END_DECLS	}
# else
#  define __BEGIN_DECLS
#  define __END_DECLS
# endif
#endif

#ifdef HAVE_BOUNDED_ATTRIBUTE
#define BOUNDED_ATTRIBUTE(t, a, b) __attribute__((__bounded__ (t,a,b)))
#else
#define BOUNDED_ATTRIBUTE(t, a, b)
#endif

#ifdef HAVE_FORMAT_ATTRIBUTE
#define FORMAT_ATTRIBUTE(t, a, b) __attribute__((__format__ (t,a,b)))
#else
#define FORMAT_ATTRIBUTE(t, a, b)
#endif

#ifdef HAVE_NONNULL_ATTRIBUTE
#define NONNULL_ATTRIBUTE(a) __attribute__((__nonnull__ (a)))
#else
#define NONNULL_ATTRIBUTE(a)
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

#define Malloc(len, t) AG_Malloc((len), (t))
#define Realloc(p, len) AG_Realloc((p), (len))

#ifdef DEBUG
#define Free(p, t) AG_Free((p), (t))
#else
/* XXX redundant on some systems */
#define Free(p, t) if ((p) != NULL) free((p))
#endif

#define Strdup(s) AG_Strdup(s)
#define Vasprintf(msg, fmt, args) do {				\
	va_start((args), (fmt));				\
	if (vasprintf((msg), (fmt), (args)) == -1) 		\
		fatal("vasprintf");				\
	va_end((args));						\
} while (0)

#include "begin_code.h"

enum {
	M_GENERIC,	/* generic */
	M_OBJECT,	/* object */
	M_POSITION,	/* object position */
	M_DEP,		/* dependency table entry */
	M_PROP,		/* property table entry */
	M_EVENT,	/* event queue entry */
	M_GFX,		/* gfx structure */
	M_AUDIO,	/* audio structure */
	M_MAP,		/* map nodes and layers */
	M_MAP_NITEM,	/* map noderefs */
	M_MAPEDIT,	/* map edition */
	M_NODEXFORM,	/* map node transform */
	M_NODEMASK,	/* map node mask */
	M_WIDGET,	/* widget */
	M_VG,		/* vector graphics */
	M_RG,		/* raster graphics */
	M_VIEW,		/* view interface */
	M_NETBUF,	/* network i/o */
	M_LOADER,	/* file loaders */
	M_TEXT,		/* text rendering */
	M_TYPESW,	/* type switch */
	M_INPUT,	/* input devices */
	M_CAD,		/* cad applications */
	M_EDA,		/* eda applications */
	M_GAME,		/* game applications */
	M_MATH,		/* math routines */
	M_SG,		/* scene graph */
	M_LAST
};

struct ag_malloc_type {
	size_t msize;			/* Total allocated memory */
	u_int nallocs;			/* Total number of allocations */
	u_int nfrees;			/* Total number of frees */
};

#ifdef DEBUG
extern int agDebugLvl;
#endif

__BEGIN_DECLS
void		 AG_InitError(void);
void		 AG_DestroyError(void);
__inline__ char	*AG_Strdup(const char *);
const char	*AG_GetError(void);
void		 AG_SetError(const char *, ...)
		     FORMAT_ATTRIBUTE(printf, 1, 2)
		     NONNULL_ATTRIBUTE(1);
void		 AG_FatalError(const char *, ...)
		     FORMAT_ATTRIBUTE(printf, 1, 2)
		     NONNULL_ATTRIBUTE(1);
void		 AG_DebugPrintf(const char *, ...)
		     FORMAT_ATTRIBUTE(printf, 1, 2)
		     NONNULL_ATTRIBUTE(1);
void		 AG_DebugPrintfNop(const char *, ...)
		     FORMAT_ATTRIBUTE(printf, 1, 2)
		     NONNULL_ATTRIBUTE(1);
void		 AG_Debug(int, const char *, ...)
		     FORMAT_ATTRIBUTE(printf, 2, 3)
		     NONNULL_ATTRIBUTE(2);
void		 AG_DebugNop(int, const char *, ...)
		     FORMAT_ATTRIBUTE(printf, 2, 3)
		     NONNULL_ATTRIBUTE(2);
void		 AG_DebugN(int, const char *, ...)
		     FORMAT_ATTRIBUTE(printf, 2, 3)
		     NONNULL_ATTRIBUTE(2);
void		*AG_PtrMismatch(void);
void		*AG_ObjectMismatch(const char *, const char *);
int		 AG_IntMismatch(void);
float		 AG_FloatMismatch(void);
__inline__ void	*AG_Malloc(size_t, int);
__inline__ void	*AG_Realloc(void *, size_t);
void		 AG_Free(void *, int);
__END_DECLS

#include "close_code.h"

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

#endif /* _AGAR_CORE_ERROR_H_ */
