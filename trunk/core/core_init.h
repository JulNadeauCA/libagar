/*	$Csoft$	*/
/*	Public domain	*/

#include "begin_code.h"

extern const char *agProgName;
extern AG_Object *agWorld;
extern AG_Mutex agLinkageLock;
extern AG_Mutex agTimingLock;

#define AG_LockLinkage() AG_MutexLock(&agLinkageLock)
#define AG_UnlockLinkage() AG_MutexUnlock(&agLinkageLock)
#define AG_LockTiming() AG_MutexLock(&agTimingLock)
#define AG_UnlockTiming() AG_MutexUnlock(&agTimingLock)

#define AG_VIDEO_HWSURFACE	0x001
#define AG_VIDEO_ASYNCBLIT	0x002
#define AG_VIDEO_ANYFORMAT	0x004
#define AG_VIDEO_HWPALETTE	0x008
#define AG_VIDEO_DOUBLEBUF	0x010
#define AG_VIDEO_FULLSCREEN	0x020
#define AG_VIDEO_RESIZABLE	0x040
#define AG_VIDEO_NOFRAME	0x080
#define AG_VIDEO_BGPOPUPMENU	0x100
#define AG_VIDEO_OPENGL		0x200
#define AG_VIDEO_OPENGL_OR_SDL	0x400

#define AG_FORCE_UNICODE	0x01
#define AG_FORCE_JOYSTICK	0x02

#define AG_INIT_DEBUG_SERVER	0x01
#define AG_INIT_RCS		0x02

#define AG_CONFIG_FULLSCREEN	0x01
#define AG_CONFIG_GL		0x02
#define AG_CONFIG_RESOLUTION	0x04
#define AG_CONFIG_DIRECTORIES	0x08
#define AG_CONFIG_ALL		0xff

__BEGIN_DECLS
int	 AG_InitCore(const char *, u_int);
int	 AG_InitVideo(int, int, int, u_int);
int	 AG_InitInput(u_int);
int	 AG_InitNetwork(u_int);
int	 AG_InitConfigWin(u_int);
void	 AG_AtExitFunc(void (*)(void));
void	 AG_Quit(void);
void	 AG_Destroy(void);
__END_DECLS

#include "close_code.h"
