/*	Public domain	*/

#ifndef _AGAR_GUI_GUI_H_
#define _AGAR_GUI_GUI_H_

#include <agar/config/have_sdl.h>

#include <agar/gui/widget.h>
#include <agar/gui/window.h>
#include <agar/gui/text.h>
#include <agar/gui/iconmgr.h>

#include <agar/gui/begin.h>

/* Flags for AG_InitVideo() */
#define AG_VIDEO_HWSURFACE     0x0001  /* Request hardware FB */
#define AG_VIDEO_ASYNCBLIT     0x0002  /* Multithreaded blits */
#define AG_VIDEO_ANYFORMAT     0x0004  /* Disable depth emulation */
#define AG_VIDEO_HWPALETTE     0x0008  /* Exclusive palette access */
#define AG_VIDEO_DOUBLEBUF     0x0010  /* Double buffering */
#define AG_VIDEO_FULLSCREEN    0x0020  /* Start in fullscreen mode */
#define AG_VIDEO_RESIZABLE     0x0040  /* Request resizable window */
#define AG_VIDEO_NOFRAME       0x0080  /* Request frameless window */
#define AG_VIDEO_BGPOPUPMENU   0x0100  /* Set a background popup menu */
#define AG_VIDEO_OPENGL	       0x0200  /* Require OpenGL mode */
#define AG_VIDEO_OPENGL_OR_SDL 0x0400  /* Prefer OpenGL mode */
#define AG_VIDEO_NOBGCLEAR     0x0800  /* Don't clear background on init */
#define AG_VIDEO_OVERLAY       0x1000  /* Overlay in OpenGL mode */
#define AG_VIDEO_SDL           0x2000  /* Prefer SDL mode */

/* Window iterators. */
#define AG_FOREACH_WINDOW(var, ob) \
	AGOBJECT_FOREACH_CHILD(var, ob, ag_window)
#define AG_FOREACH_WINDOW_REVERSE(var, ob) \
	AGOBJECT_FOREACH_CHILD_REVERSE(var, ob, ag_window)

__BEGIN_DECLS
extern const char *agBlendFuncNames[];	/* For enum ag_blend_func */

extern int agGUI;			/* GUI is initialized */

int        AG_InitGraphics(const char *);
int        AG_InitVideo(int, int, int, Uint);
#ifdef HAVE_SDL
int        AG_InitVideoSDL(void *, Uint);
#endif
int        AG_InitGUI(Uint);
int        AG_InitGUIGlobals(void);
void       AG_DestroyGUI(void);
void       AG_DestroyGUIGlobals(void);
void       AG_QuitGUI(void);
void       AG_DestroyVideo(void);
#ifdef AG_DEBUG
AG_Window *AG_GuiDebugger(void);
#endif

#ifdef AG_LEGACY
#define AGVIEW_FOREACH_WINDOW AG_FOREACH_WINDOW
#define AG_SDLKEY(v) ((SDLKey)AG_INT(v))
#define AG_SDLMOD(v) ((SDLMod)AG_INT(v))
#endif /* AG_LEGACY */
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_GUI_H_ */
