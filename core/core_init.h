/*	Public domain	*/

#include "begin_code.h"

__BEGIN_DECLS
struct ag_event;

extern char *agProgName;	/* User program name */
extern int agVerbose;		/* Verbose console output */
extern int agTerminating;	/* Application is exiting */
extern int agGUI;		/* GUI is initialized */
extern int agInitedSDL;		/* Video system had to initialize SDL */

/* Flags for AG_InitCore() */
#define AG_CORE_VERBOSE		0x01

int	 AG_InitCore(const char *, Uint);
int	 AG_InitVideo(int, int, int, Uint);
int	 AG_InitNetwork(Uint);
void	 AG_AtExitFunc(void (*)(void));
void	 AG_AtExitFuncEv(void (*)(struct ag_event *));
void	 AG_Quit(void);
void	 AG_Destroy(void);

/* Legacy */
#define AG_InitInput(flags)
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

#include "close_code.h"
