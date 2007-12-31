/*	Public domain	*/

#include "begin_code.h"

__BEGIN_DECLS
struct ag_object;
struct ag_event;

extern const char *agProgName;
extern int agVerbose;
extern int agTerminating;

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

#include "close_code.h"
