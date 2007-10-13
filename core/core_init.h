/*	Public domain	*/

#include "begin_code.h"

#define AG_LockLinkage() AG_MutexLock(&agLinkageLock)
#define AG_UnlockLinkage() AG_MutexUnlock(&agLinkageLock)
#define AG_LockTiming() AG_MutexLock(&agTimingLock)
#define AG_UnlockTiming() AG_MutexUnlock(&agTimingLock)

/* Flags for AG_InitCore() */
#define AG_CORE_VERBOSE		0x01

/* Flags for AG_InitInput() */
#define AG_FORCE_UNICODE	0x01
#define AG_FORCE_JOYSTICK	0x02

__BEGIN_DECLS
extern const char *agProgName;
extern AG_Object *agWorld;
extern AG_Mutex agLinkageLock;
extern AG_Mutex agTimingLock;
extern int agVerbose;
extern int agTerminating;

int	 AG_InitCore(const char *, Uint);
int	 AG_InitVideo(int, int, int, Uint);
int	 AG_InitInput(Uint);
int	 AG_InitNetwork(Uint);
void	 AG_AtExitFunc(void (*)(void));
void	 AG_AtExitFuncEv(void (*)(AG_Event *));
void	 AG_Quit(void);
void	 AG_Destroy(void);
__END_DECLS

#include "close_code.h"
