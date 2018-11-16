/*	Public domain	*/

#ifndef	_AGAR_CORE_ERROR_H_
#define	_AGAR_CORE_ERROR_H_

#include <agar/config/_mk_have_stdlib_h.h>
#ifdef _MK_HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <agar/core/begin.h>

/* Standard error code */
typedef enum ag_error_code {
	AG_EUNDEFINED,		/* Undefined error */
	AG_EPERM,		/* Operation not permitted */
	AG_ENOENT,		/* No such file or directory */
	AG_EINTR,		/* Interrupted system call */
	AG_EIO,			/* Input/output error */
	AG_E2BIG,		/* Argument list too long */
	AG_EACCESS,		/* Permission denied */
	AG_EBUSY,		/* Device or resource busy */
	AG_EEXIST,		/* File exists */
	AG_ENOTDIR,		/* Not a directory */
	AG_EISDIR,		/* Is a directory */
	AG_EMFILE,		/* Too many open files */
	AG_EFBIG,		/* File too large */
	AG_ENOSPC,		/* No space left on device */
	AG_EROFS,		/* Read-only file system */
	AG_EAGAIN		/* Resource temporarily unavailable */
} AG_ErrorCode;

#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_STD)
# define Malloc(len) AG_Malloc(len)
# define TryMalloc(len) AG_TryMalloc(len)
# define Free(p) AG_Free(p)
# define Realloc(p,len) AG_Realloc((p),(len))
# define TryRealloc(p,len) AG_TryRealloc((p),(len))
# define Verbose AG_Verbose
# ifdef AG_DEBUG
#  define Debug AG_Debug
# else
#  ifdef __GNUC__
#   define Debug(obj, arg...) ((void)0)
#  else
#   define Debug AG_Debug
#  endif
# endif /* AG_DEBUG */
#endif /* _AGAR_INTERNAL or _USE_AGAR_STD */

__BEGIN_DECLS
extern int agDebugLvl;

int  AG_InitErrorSubsystem(void);
void AG_DestroyErrorSubsystem(void);

AG_ErrorCode AG_GetErrorCode(void) _Pure_Attribute_If_Unthreaded;
void         AG_SetErrorCode(AG_ErrorCode);

const char *_Nullable AG_Strerror(int);
const char *_Nonnull AG_GetError(void) _Pure_Attribute_If_Unthreaded;

void AG_SetError(const char *_Nonnull, ...)
                FORMAT_ATTRIBUTE(printf,1,2);

void AG_SetErrorS(const char *_Nonnull);

void AG_FatalError(const char *_Nullable)
                  _Noreturn_Attribute;

void AG_SetFatalCallback(void (*_Nullable)(const char *_Nonnull));
void AG_SetVerboseCallback(int (*_Nullable)(const char *_Nonnull));
void AG_SetDebugCallback(int (*_Nullable)(const char *_Nonnull));

void AG_Debug(void *_Nullable, const char *_Nonnull, ...)
             FORMAT_ATTRIBUTE(printf,2,3);

void AG_Verbose(const char *_Nonnull, ...)
               FORMAT_ATTRIBUTE(printf,1,2);

#ifdef AG_TYPE_SAFETY
void *_Nullable AG_PtrMismatch(void)    _Noreturn_Attribute;
char *_Nonnull AG_StringMismatch(void)  _Noreturn_Attribute;
void *_Nullable AG_ObjectMismatch(void) _Noreturn_Attribute;
int    AG_IntMismatch(void)             _Noreturn_Attribute;
long   AG_LongMismatch(void)            _Noreturn_Attribute;
# ifdef AG_HAVE_FLOAT
float  AG_FloatMismatch(void)           _Noreturn_Attribute;
double AG_DoubleMismatch(void)          _Noreturn_Attribute;
#  ifdef AG_HAVE_LONG_DOUBLE
long double AG_LongDoubleMismatch(void) _Noreturn_Attribute;
#  endif
# endif
#endif /* AG_TYPE_SAFETY */

/* Malloc wrapper which raise an exception on failure. */
static __inline__ void *_Nonnull _Malloc_Like_Attribute
AG_Malloc(AG_Size len)
{
	void *p;

	if ((p = malloc(len)) == NULL) {
		AG_FatalError("malloc");
	}
	return (p);
}

/* Malloc wrapper which returns NULL on failure. */
static __inline__ void *_Nullable _Malloc_Like_Attribute
AG_TryMalloc(AG_Size len)
{
	void *p;
	if ((p = malloc(len)) == NULL) {
		AG_SetErrorS("Out of memory");
		return (NULL);
	}
	return (p);
}

/* Realloc wrapper which raise an exception on failure. */
static __inline__ void *_Nonnull
AG_Realloc(void *_Nullable pOld, AG_Size len)
{
	void *p;
	if ((p = realloc(pOld, len)) == NULL) {
		AG_FatalError("realloc");
	}
	return (p);
}

/* Realloc wrapper which returns NULL on failure. */
static __inline__ void *_Nullable
AG_TryRealloc(void *_Nullable pOld, AG_Size len)
{
	void *p;
	
	if ((p = realloc(pOld, len)) == NULL) {
		AG_SetErrorS("Out of memory");
	}
	return (p);
}

/* Free wrapper for symmetry */
#define AG_Free(p) free(p)
__END_DECLS

#include <agar/core/close.h>
#endif /* _AGAR_CORE_ERROR_H_ */
