/*	Public domain	*/

#ifndef	_AGAR_CORE_CORE_INIT_H_
#define	_AGAR_CORE_CORE_INIT_H_
#include <agar/core/begin.h>

/* Flags for AG_InitCore() */
#define AG_VERBOSE         0x01 /* Allow errors/warning output on console */
#define AG_CREATE_DATADIR  0x02 /* Auto-create data directory on init */
#define AG_SOFT_TIMERS	   0x04	/* Do not use hardware timers. A custom event
				   loop will call AG_ProcessTimeouts(). */

__BEGIN_DECLS
struct ag_event;

extern char *agProgName;	/* User program name */
extern int agVerbose;		/* Verbose console output */
extern int agSoftTimers;	/* Disable hardware timers */

int	 AG_InitCore(const char *, Uint);
void	 AG_AtExitFunc(void (*)(void));
void	 AG_AtExitFuncEv(void (*)(struct ag_event *));
void	 AG_Quit(void) NORETURN_ATTRIBUTE;
void	 AG_Destroy(void);

#ifdef AG_LEGACY
# define AG_InitInput(flags)
# define AG_CORE_VERBOSE AG_VERBOSE
# define AG_NO_CFG_AUTOLOAD 0x04	/* Former AG_InitCore() option */
#endif /* AG_LEGACY */
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
