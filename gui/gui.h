/*	Public domain	*/

#ifndef _AGAR_GUI_GUI_H_
#define _AGAR_GUI_GUI_H_

#include <agar/config/have_sdl.h>
#include <agar/gui/begin.h>

#define AG_ZOOM_MIN 0
#define AG_ZOOM_MAX 20
#define AG_ZOOM_1_1 8		/* 12.5% to 100% (in 8 increments) */
#ifndef AG_ZOOM_DEFAULT
#define AG_ZOOM_DEFAULT AG_ZOOM_1_1
#endif

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
extern int agGUI, agRenderingContext, agStereo, agXsync, agClipboardIntegration;
extern int agKbdDelay, agKbdRepeat;
extern int agMouseDblclickDelay, agMouseSpinDelay, agMouseSpinIval,
           agMouseScrollIval, agScrollButtonIval, agPageIncrement,
           agAutocompleteDelay, agAutocompleteRate, agScreenshotQuality;
extern int agTextComposition, agTextTabWidth, agTextBlinkRate;
extern int agGLdebugOutput, agGLuseNPOT;
extern double agZoomValues[AG_ZOOM_MAX];

#ifdef AG_EVENT_LOOP
void  AG_QuitGUI(void);
#endif
int   AG_InitGraphics(const char *_Nullable);
void  AG_DestroyGraphics(void);
void  AG_ZoomIn(void);
void  AG_ZoomOut(void);
void  AG_ZoomReset(void);
int   AG_InitGUI(Uint);
void  AG_DestroyGUI(void);
int   AG_InitGUIGlobals(void);
void  AG_DestroyGUIGlobals(void);

#ifdef HAVE_SDL
int   AG_InitVideoSDL(void *_Nonnull, Uint);
int   AG_SetVideoSurfaceSDL(void *_Nonnull);
#endif

#ifdef AG_WIDGETS
void                        AG_About(struct ag_event *_Nullable);
struct ag_window *_Nullable AG_StyleEditor(struct ag_window *_Nonnull);
void                        AG_StyleEditorDetachTarget(void);
void                        AG_StyleEditorDetachWindow(void);
# ifdef AG_DEBUG
struct ag_window *_Nullable AG_GuiDebugger(struct ag_window *_Nonnull);
void                        AG_GuiDebuggerDetachTarget(void);
void                        AG_GuiDebuggerDetachWindow(void);
# endif
void                        AG_DEV_ConfigShow(void);
void *_Nullable             AG_DEV_ObjectEdit(void *_Nonnull);
# ifdef AG_TIMERS
struct ag_window *_Nullable AG_DEV_Browser(void *_Nonnull);
struct ag_window *_Nullable AG_DEV_ClassInfo(void);
struct ag_window *_Nullable AG_DEV_FontInfo(void);
#  ifdef AG_ENABLE_STRING
struct ag_window *_Nullable AG_DEV_TimerInspector(void);
#  endif
# endif
# ifdef AG_UNICODE
struct ag_window *_Nullable AG_DEV_UnicodeBrowser(void);
# endif
struct ag_window *_Nullable AG_DEV_CPUInfo(void);
#endif /* AG_WIDGETS */

#ifdef AG_LEGACY
int  AG_InitVideo(int,int, int, Uint) DEPRECATED_ATTRIBUTE;
void AG_DestroyVideo(void) DEPRECATED_ATTRIBUTE;
#endif
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_GUI_H_ */
