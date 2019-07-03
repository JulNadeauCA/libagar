/*	Public domain	*/

#ifndef _AGAR_GUI_GUI_H_
#define _AGAR_GUI_GUI_H_

#include <agar/config/have_sdl.h>
#include <agar/gui/begin.h>

#ifdef AG_HAVE_FLOAT
# define AG_ZOOM_RANGE 20
#else
# define AG_ZOOM_RANGE 10
#endif

#define AG_ZOOM_DEFAULT	8		/* Initial zoom setting (1.0) */

/* Flags for AG_InitVideoSDL() */
#define AG_VIDEO_HWSURFACE     0x0001
#define AG_VIDEO_ASYNCBLIT     0x0002
#define AG_VIDEO_ANYFORMAT     0x0004
#define AG_VIDEO_HWPALETTE     0x0008
#define AG_VIDEO_DOUBLEBUF     0x0010
#define AG_VIDEO_FULLSCREEN    0x0020
#define AG_VIDEO_RESIZABLE     0x0040
#define AG_VIDEO_NOFRAME       0x0080
#define AG_VIDEO_BGPOPUPMENU   0x0100
#define AG_VIDEO_OPENGL	       0x0200
#define AG_VIDEO_OPENGL_OR_SDL 0x0400
#define AG_VIDEO_NOBGCLEAR     0x0800
#define AG_VIDEO_OVERLAY       0x1000
#define AG_VIDEO_SDL           0x2000

__BEGIN_DECLS
extern int agGUI, agRenderingContext, agStereo, agXsync;
extern int agKbdDelay, agKbdRepeat;
extern int agMouseDblclickDelay, agMouseSpinDelay, agMouseSpinIval,
           agMouseScrollDelay, agMouseScrollIval, agPageIncrement;
#ifdef AG_UNICODE
extern int agTextComposition, agTextBidi;
#endif
extern int agTextCache, agTextTabWidth, agTextBlinkRate, agTextSymbols;
extern int agScreenshotQuality;
#ifdef AG_HAVE_FLOAT
extern double agZoomValues[AG_ZOOM_RANGE];
#else
extern int    agZoomValues[AG_ZOOM_RANGE];
#endif

int   AG_InitGraphics(const char *_Nullable);
void  AG_DestroyGraphics(void);
int   AG_InitGUI(Uint);
void  AG_DestroyGUI(void);
int   AG_InitGUIGlobals(void);
void  AG_DestroyGUIGlobals(void);
void  AG_QuitGUI(void);
void  AG_ZoomIn(void);
void  AG_ZoomOut(void);
void  AG_ZoomReset(void);

#if defined(AG_DEBUG) && defined(AG_TIMERS)
void *_Nullable AG_GuiDebugger(void *_Nullable);
#endif

#ifdef HAVE_SDL
int   AG_InitVideoSDL(void *_Nonnull, Uint);
int   AG_SetVideoSurfaceSDL(void *_Nonnull);
#endif

#ifdef AG_LEGACY
int  AG_InitVideo(int,int, int, Uint) DEPRECATED_ATTRIBUTE;
void AG_DestroyVideo(void) DEPRECATED_ATTRIBUTE;
#endif
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_GUI_H_ */
