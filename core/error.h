/*	Public domain	*/

#ifndef	_AGAR_CORE_ERROR_H_
#define	_AGAR_CORE_ERROR_H_
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
# define Malloc(len)       AG_Malloc(len)
# define TryMalloc(len)    AG_TryMalloc(len)
# define Free(p)           AG_Free(p)
# define Realloc(p,len)    AG_Realloc((p),(len))
# define TryRealloc(p,len) AG_TryRealloc((p),(len))

# ifdef AG_VERBOSITY
#  define Verbose AG_Verbose
# else
#  ifdef __GNUC__
#   define Verbose(obj, arg...) ((void)0)
#  else
#   define Verbose AG_Verbose
#  endif
# endif

# ifdef AG_DEBUG
#  define Debug AG_Debug
#  define Debug_Mute(x)   x=agDebugLvl; agDebugLvl=0
#  define Debug_Unmute(x) agDebugLvl=x
# else
#  if defined(__GNUC__)
#   define Debug(obj, arg...) ((void)0)
#  else
#   define Debug AG_Debug
#  endif
#  define Debug_Mute(x)
#  define Debug_Unmute(x)
# endif /* AG_DEBUG */

#endif /* _AGAR_INTERNAL or _USE_AGAR_STD */

__BEGIN_DECLS
extern int agDebugLvl;

int  AG_InitErrorSubsystem(void);
void AG_DestroyErrorSubsystem(void);

AG_ErrorCode AG_GetErrorCode(void) _Pure_Attribute_If_Unthreaded;
void         AG_SetErrorCode(AG_ErrorCode);

const char *_Nullable AG_Strerror(int);
const char *_Nonnull  AG_GetError(void) _Pure_Attribute_If_Unthreaded;

void AG_SetErrorS(const char *_Nonnull);
void AG_SetError(const char *_Nonnull, ...)
                FORMAT_ATTRIBUTE(printf,1,2);

void AG_FatalError(const char *_Nullable) _Noreturn_Attribute;
void AG_FatalErrorF(const char *_Nullable, ...)
                   FORMAT_ATTRIBUTE(printf,1,2) _Noreturn_Attribute;

#ifdef AG_VERBOSITY
# define AG_SetErrorV(c,s)   AG_SetErrorS(s)
# define AG_FatalErrorV(c,s) AG_FatalError(s)
#else
# define AG_SetErrorV(c,s)   AG_SetErrorS(c)
# define AG_FatalErrorV(c,s) AG_FatalError(c)
#endif

void AG_SetFatalCallback(void (*_Nullable)(const char *_Nonnull));
void AG_SetVerboseCallback(int (*_Nullable)(const char *_Nonnull));
void AG_SetDebugCallback(int (*_Nullable)(const char *_Nonnull));

void AG_Debug(void *_Nullable, const char *_Nonnull, ...)
             FORMAT_ATTRIBUTE(printf,2,3);

void AG_Verbose(const char *_Nonnull, ...)
               FORMAT_ATTRIBUTE(printf,1,2);

#ifdef AG_TYPE_SAFETY
void *_Nullable AG_GenericMismatch(const char *_Nonnull) _Noreturn_Attribute;
void *_Nullable AG_PtrMismatch(void) _Noreturn_Attribute;
char *_Nonnull  AG_StringMismatch(void) _Noreturn_Attribute;
void *_Nullable AG_ObjectMismatch(void) _Noreturn_Attribute;
int             AG_IntMismatch(void) _Noreturn_Attribute;
long            AG_LongMismatch(void) _Noreturn_Attribute;
# ifdef AG_HAVE_FLOAT
float           AG_FloatMismatch(void) _Noreturn_Attribute;
double          AG_DoubleMismatch(void) _Noreturn_Attribute;
# endif
#endif /* AG_TYPE_SAFETY */

/*
 * Inlinables
 */
void *_Nonnull  ag_malloc(AG_Size) _Malloc_Like_Attribute;
void *_Nonnull  ag_try_malloc(AG_Size) _Malloc_Like_Attribute;
void *_Nonnull  ag_realloc(void *_Nullable, AG_Size);
void *_Nullable ag_try_realloc(void *_Nullable, AG_Size);
void            ag_free(void *_Nullable);
#ifdef AG_INLINE_ERROR
# define AG_INLINE_HEADER
# include <agar/core/inline_error.h>
#else
# define AG_Malloc(s)       ag_malloc(s)
# define AG_TryMalloc(s)    ag_try_malloc(s)
# define AG_Realloc(p,s)    ag_realloc((p),(s))
# define AG_TryRealloc(p,s) ag_try_realloc((p),(s))
# define AG_Free(p)         ag_free(p)
#endif /* !AG_INLINE_ERROR */
__END_DECLS

#include <agar/core/close.h>
#endif /* _AGAR_CORE_ERROR_H_ */
