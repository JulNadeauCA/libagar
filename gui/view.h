/*	Public domain	*/

#ifndef _AGAR_GUI_VIEW_H_
#define _AGAR_GUI_VIEW_H_

#include <agar/config/have_opengl.h>
#include <agar/config/view_8bpp.h>
#include <agar/config/view_16bpp.h>
#include <agar/config/view_24bpp.h>
#include <agar/config/view_32bpp.h>

#include <agar/gui/begin.h>

struct ag_window;
struct ag_style;
AG_TAILQ_HEAD(ag_windowq, ag_window);

enum ag_wm_operation {
	AG_WINOP_NONE,		/* No operation */
	AG_WINOP_MOVE,		/* Move window */
	AG_WINOP_LRESIZE,	/* Resize (via left control) */
	AG_WINOP_RRESIZE,	/* Resize (via right control) */
	AG_WINOP_HRESIZE	/* Resize (via horizontal control) */
};

typedef struct ag_display {
	struct ag_object obj;

	SDL_Surface *v;			/* Video surface */
	AG_Surface *stmpl;		/* Reference surface */
	int w, h;			/* Display geometry */
	int depth;			/* Depth in bpp */
	int opengl;			/* OpenGL rendering? (if available) */
	int rCur;			/* Estimated refresh delay in ms */
	Uint rNom;			/* Nominal refresh delay */
	SDL_Rect *dirty;		/* Video rectangles to update */
	Uint	 ndirty;
	Uint  maxdirty;
	
	struct ag_windowq detach;	/* Window detach queue */
	struct ag_window *winToFocus;	/* Give focus to this window,
					   when event processing is done */
	struct ag_window *winSelected;	/* Window being moved/resized/etc */
	AG_List *Lmodal;		/* Modal window stack */
	enum ag_wm_operation winop;	/* WM operation in progress */
	struct ag_style *style;		/* Default style for new windows */
	int overlay;			/* GL overlay mode */
} AG_Display;

/*
 * Putpixel macros optimized for the display surface format.
 */

#ifdef VIEW_8BPP
# define AG_VIEW_PUT_PIXEL8(dst, c)	\
case 1:					\
	*(dst) = (c);			\
	break;
#else
# define AG_VIEW_PUT_PIXEL8(dst, c)
#endif

#ifdef VIEW_16BPP
# define AG_VIEW_PUT_PIXEL16(dst, c)	\
case 2:					\
	*(Uint16 *)(dst) = (c);		\
	break;
#else
# define AG_VIEW_PUT_PIXEL16(dst, c)
#endif

#ifdef VIEW_24BPP
# if AG_BYTEORDER == AG_BIG_ENDIAN
#  define AG_VIEW_PUT_PIXEL24(dst, c)	\
case 3:					\
	(dst)[0] = ((c)>>16) & 0xff;	\
	(dst)[1] = ((c)>>8) & 0xff;	\
	(dst)[2] =  (c) & 0xff;		\
	break;
# else
#  define AG_VIEW_PUT_PIXEL24(dst, c)	\
case 3:					\
	(dst)[0] =  (c) & 0xff;		\
	(dst)[1] = ((c)>>8) & 0xff;	\
	(dst)[2] = ((c)>>16) & 0xff;	\
	break;
# endif
#else
# define AG_VIEW_PUT_PIXEL24(dst, c)
#endif

#ifdef VIEW_32BPP
# define AG_VIEW_PUT_PIXEL32(dst, c)	\
case 4:					\
	*(Uint32 *)(dst) = (c);		\
	break;
#else
# define AG_VIEW_PUT_PIXEL32(dst, c)
#endif

#define AG_VIEW_PUT_PIXEL(p, c) do {					\
	switch (agVideoFmt->BytesPerPixel) {				\
		AG_VIEW_PUT_PIXEL8((p),  (c))				\
		AG_VIEW_PUT_PIXEL16((p), (c))				\
		AG_VIEW_PUT_PIXEL24((p), (c))				\
		AG_VIEW_PUT_PIXEL32((p), (c))				\
	}								\
} while (0)
#define AG_VIEW_PUT_PIXEL2(vx, vy, c) do {				\
	Uint8 *_view_dst = (Uint8 *)agView->v->pixels +			\
	    (vy)*agView->v->pitch + (vx)*agVideoFmt->BytesPerPixel;	 \
	switch (agVideoFmt->BytesPerPixel) {				 \
		AG_VIEW_PUT_PIXEL8(_view_dst,  (c))			\
		AG_VIEW_PUT_PIXEL16(_view_dst, (c))			\
		AG_VIEW_PUT_PIXEL24(_view_dst, (c))			\
		AG_VIEW_PUT_PIXEL32(_view_dst, (c))			\
	}								\
} while (0)
#define AG_VIEW_PUT_PIXEL2_CLIPPED(vx, vy, c) do {			\
	if (!AG_CLIPPED_PIXEL(agView->v, (vx), (vy))) {			\
		Uint8 *_view_dst = (Uint8 *)agView->v->pixels +		\
		    (vy)*agView->v->pitch + (vx)*agVideoFmt->BytesPerPixel; \
		switch (agVideoFmt->BytesPerPixel) {			 \
			AG_VIEW_PUT_PIXEL8(_view_dst,  (c))		\
			AG_VIEW_PUT_PIXEL16(_view_dst, (c))		\
			AG_VIEW_PUT_PIXEL24(_view_dst, (c))		\
			AG_VIEW_PUT_PIXEL32(_view_dst, (c))		\
		}							\
	}								\
} while (0)

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

#define AGVIEW_FOREACH_WINDOW(var, ob) \
	AGOBJECT_FOREACH_CHILD(var, ob, ag_window)
#define AGVIEW_FOREACH_WINDOW_REVERSE(var, ob) \
	AGOBJECT_FOREACH_CHILD_REVERSE(var, ob, ag_window)

#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_GUI)
#define VIEW_FOREACH_WINDOW(var,ob)		AGVIEW_FOREACH_WINDOW(var,ob)
#define VIEW_FOREACH_WINDOW_REVERSE(var,ob)	AGVIEW_FOREACH_WINDOW_REVERSE(var,ob)
#endif

__BEGIN_DECLS
extern AG_ObjectClass agDisplayClass;	/* Agar(Display) class definition */

extern AG_Display     *agView;		/* Main Display */
extern AG_PixelFormat *agVideoFmt;	/* Main Display pixel format */
extern AG_PixelFormat *agSurfaceFmt;	/* Preferred format for surfaces */

extern int         agRenderingContext;	/* Running in rendering context? */
extern const char *agBlendFuncNames[];	/* For enum ag_blend_func */

extern AG_ClipRect *agClipRects;	/* Clipping rectangle stack (first
					   entry always covers whole view */
extern int          agClipStateGL[4];	/* Saved GL clipping plane states */
extern Uint         agClipRectCount;

extern const SDL_VideoInfo *agVideoInfo; /* XXX */

extern int agFullscreenMode;		/* Full-screen mode is effective */
extern int agAsyncBlits;		/* Async blits are effective */
extern int agGUI;			/* GUI is initialized */
extern int agInitedSDL;			/* SDL_Init() was used */
extern int agInitedSDLVideo;		/* SDL_INIT_VIDEO was used */

int  AG_InitVideo(int, int, int, Uint);
int  AG_InitVideoSDL(SDL_Surface *, Uint);
int  AG_InitGUI(Uint);
void AG_DestroyGUI(void);
void AG_QuitGUI(void);
void AG_ClearBackground(void);
int  AG_SetRefreshRate(int);
void AG_ViewUpdateFB(const AG_Rect2 *);
void AG_ViewVideoExpose(void);
int  AG_ResizeDisplay(int, int);
void AG_DestroyVideo(void);
void AG_BeginRendering(void);
void AG_EndRendering(void);
int  AG_ProcessEvent(SDL_Event *);
void AG_SetVideoResizeCallback(void (*)(Uint, Uint));

void AG_BindGlobalKey(SDLKey, SDLMod, void (*)(void));
void AG_BindGlobalKeyEv(SDLKey, SDLMod, void (*)(AG_Event *));
int  AG_UnbindGlobalKey(SDLKey, SDLMod);
void AG_ClearGlobalKeys(void);

void        AG_ViewCapture(void);
#ifdef HAVE_OPENGL
Uint        AG_SurfaceTexture(AG_Surface *, float *);
void        AG_UpdateTexture(AG_Surface *, int);
AG_Surface *AG_CaptureGLView(void);
#endif

void    AG_EventLoop_FixedFPS(void);
#define AG_EventLoop() AG_EventLoop_FixedFPS()
Uint8   AG_MouseGetState(int *, int *);

#ifdef AG_DEBUG
struct ag_window *AG_GuiDebugger(void);
#endif

#define AG_LockView()	SDL_LockSurface(agView->v)
#define AG_UnlockView()	SDL_UnlockSurface(agView->v)

/* Pre-1.4 */
#ifdef AG_LEGACY
#define AG_SDLKEY(v) ((SDLKey)AG_INT(v))
#define AG_SDLMOD(v) ((SDLMod)AG_INT(v))
#define AG_ViewAttach(win) AG_ObjectAttach(agView,(win))
#define AG_ViewDetach(win) AG_ObjectDetach(win)
#endif

/* Convert a pixel from SurfaceFmt to VideoFmt. */
static __inline__ Uint32
AG_VideoPixel(Uint32 c)
{
	Uint8 r, g, b;
	AG_GetRGB(c, agSurfaceFmt, &r,&g,&b);
	return AG_MapRGB(agVideoFmt, r,g,b);
}

/* Convert a pixel from VideoFmt to SurfaceFmt. */
static __inline__ Uint32
AG_SurfacePixel(Uint32 c)
{
	Uint8 r, g, b;
	AG_GetRGB(c, agVideoFmt, &r,&g,&b);
	return AG_MapRGB(agSurfaceFmt, r,g,b);
}
__END_DECLS

#include <agar/gui/close.h>

#include <agar/gui/iconmgr.h>
#include <agar/gui/load_surface.h>

#endif	/* _AGAR_GUI_VIEW_H_ */
