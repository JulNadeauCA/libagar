/*	Public domain	*/

#ifndef	_AGAR_CORE_CORE_INIT_H_
#define	_AGAR_CORE_CORE_INIT_H_
#include <agar/core/begin.h>

/* Flags for AG_InitCore() */
#define AG_VERBOSE        0x01 /* Allow errors and warnings on the console */
#define AG_CREATE_DATADIR 0x02 /* Check for and create app. data directory */
#define AG_SOFT_TIMERS	  0x04 /* Use a software timing wheel instead of
				  OS-provided timer facilities */
#define AG_POSIX_USERS    0x08 /* Use the AG_User(3) module "posix" instead
				  of "getenv" if both are supported */

#define AG_MEMORY_MODEL_NAME (agMemoryModelNames[AG_MODEL >> 5])

__BEGIN_DECLS
struct ag_event;

extern char *_Nullable agProgName;	/* User program name */
extern int agVerbose;			/* Verbose console output */
extern int agSoftTimers;		/* Disable hardware timers */
extern const char *agMemoryModelNames[];

int  AG_InitCore(const char *_Nullable, Uint);
void AG_AtExitFunc(void (*_Nullable)(void));
void AG_AtExitFuncEv(void (*_Nullable)(struct ag_event *_Nonnull));
void AG_Destroy(void);
void AG_Quit(void) _Noreturn_Attribute;
__END_DECLS

/* Utility macros */
#define AG_SETFLAGS(var,flags,cond)		\
	do {					\
		if (cond) {			\
			(var) |= (flags);	\
		} else {			\
			(var) &= ~(flags);	\
		}				\
	} while (0)

#define AG_INVFLAGS(var,flags)			\
	do {					\
		if ((var) & (flags)) {		\
			(var) &= ~(flags);	\
		} else {			\
			(var) |= (flags);	\
		}				\
	} while (0)

#ifndef AG_MIN
#define	AG_MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef AG_MAX
#define	AG_MAX(a,b) (((a)>(b))?(a):(b))
#endif
#ifndef AG_MIN3
#define	AG_MIN3(a,b,c) AG_MIN((a),AG_MIN((b),(c)))
#endif
#ifndef AG_MAX3
#define	AG_MAX3(a,b,c) AG_MAX((a),AG_MAX((b),(c)))
#endif

#if defined(_WIN32) || defined(_XBOX)
# define AG_PATHSEP "\\"
# define AG_PATHSEPCHAR '\\'
# define AG_PATHSEPMULTI ";"
#else
# define AG_PATHSEP "/"
# define AG_PATHSEPCHAR '/'
# define AG_PATHSEPMULTI ":"
#endif

#include <agar/core/close.h>
#endif /* _AGAR_CORE_CORE_INIT_H_ */
