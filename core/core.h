/*	$Csoft: engine.h,v 1.102 2005/10/02 09:41:08 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_CORE_H_
#define _AGAR_CORE_H_

#include <agar/config/have_opengl.h>
#include <agar/config/enable_nls.h>
#include <agar/config/threads.h>
#include <agar/config/floating_point.h>
#include <agar/config/edition.h>
#include <agar/config/network.h>
#include <agar/config/have_bounded_attribute.h>
#include <agar/config/have_format_attribute.h>
#include <agar/config/have_nonnull_attribute.h>
#include <agar/config/map.h>

#include <sys/types.h>

#include <SDL.h>
#include <SDL_endian.h>
#include <SDL_cpuinfo.h>

#ifdef HAVE_OPENGL
# ifdef __APPLE__
#  include <OpenGL/gl.h>
# else
#  include <GL/gl.h>
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

#include <agar/core/error.h>
#include <agar/core/threads.h>

#include <agar/compat/queue.h>
#include <agar/compat/strlcpy.h>
#include <agar/compat/strlcat.h>
#include <agar/compat/snprintf.h>
#include <agar/compat/vsnprintf.h>
#include <agar/compat/asprintf.h>
#include <agar/compat/vasprintf.h>
#include <agar/compat/strsep.h>
#include <agar/compat/math.h>

#include <agar/core/loaders/netbuf.h>
#include <agar/core/loaders/integral.h>
#include <agar/core/loaders/real.h>
#include <agar/core/loaders/string.h>
#include <agar/core/loaders/version.h>
#include <agar/core/loaders/color.h>

#include <agar/core/object.h>
#include <agar/core/event.h>
#include <agar/core/icons.h>

#include <agar/gui/text.h>

#ifdef ENABLE_NLS
# include <agar/libintl/libintl.h>
# define _(String) gettext(String)
# define gettext_noop(String) (String)
# define N_(String) gettext_noop(String)
#else
# undef _
# undef N_
# undef textdomain
# undef bindtextdomain
# define _(s) (s)
# define N_(s) (s)
# define textdomain(d)
# define bindtextdomain(p, d)
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

#ifndef MIN
#define	MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define	MAX(a,b) (((a)>(b))?(a):(b))
#endif
#ifndef MIN3
#define	MIN3(a,b,c) MIN((a),MIN((b),(c)))
#endif
#ifndef MAX3
#define	MAX3(a,b,c) MAX((a),MAX((b),(c)))
#endif

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
Uint8	 AG_MouseGetState(int *, int *);
void	 AG_AtExitFunc(void (*)(void));
void	 AG_Quit(void);
void	 AG_Destroy(void);
__END_DECLS

#include "close_code.h"
#endif	/* !_AGAR_CORE_H_ */
