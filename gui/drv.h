/*	Public domain	*/
/*
 * Generic graphics/input driver framework.
 */

#ifndef _AGAR_GUI_DRV_H_
#define _AGAR_GUI_DRV_H_

#include <agar/gui/mouse.h>
#include <agar/gui/keyboard.h>
#include <agar/gui/keymap.h>
#include <agar/gui/surface.h>

#include <agar/gui/begin.h>

enum ag_driver_type {
	AG_FRAMEBUFFER,			/* Direct rendering to frame buffer */
	AG_VECTOR			/* Vector drawing */
};
enum ag_driver_wm_type {
	AG_WM_SINGLE,			/* Single display */
	AG_WM_MULTIPLE			/* Multiple windows */
};

struct ag_widget;
struct ag_window;
struct ag_glyph;
struct ag_glyph_cache;
struct ag_cursor;
struct ag_driver_event;

/* Generic graphics driver class */
typedef struct ag_driver_class {
	struct ag_object_class _inherit;
	const char *_Nonnull name;		/* Short name */
	enum ag_driver_type type;		/* Driver type */
	enum ag_driver_wm_type wm;		/* Window manager type */
	Uint flags;
#define AG_DRIVER_OPENGL	0x01		/* Supports OpenGL calls */
#define AG_DRIVER_SDL		0x02		/* Supports SDL calls */
#define AG_DRIVER_TEXTURES	0x04		/* Support texture ops */

	/* Initialization */
	int  (*_Nonnull open)(void *_Nonnull, const char *_Nullable);
	void (*_Nonnull close)(void *_Nonnull);
	int  (*_Nonnull getDisplaySize)(Uint *_Nonnull, Uint *_Nonnull);
	
	/* Event Processing */
	void (*_Nullable beginEventProcessing)(void *_Nonnull);
	int  (*_Nonnull  pendingEvents)(void *_Nonnull);
	int  (*_Nonnull  getNextEvent)(void *_Nullable,
	                               struct ag_driver_event *_Nonnull);
	int  (*_Nonnull  processEvent)(void *_Nullable,
	                               struct ag_driver_event *_Nonnull);
	void (*_Nullable genericEventLoop)(void *_Nonnull);
	void (*_Nonnull  endEventProcessing)(void *_Nonnull);
	void (*_Nullable terminate)(void);

	/* Rendering Ops */
	void (*_Nonnull  beginRendering)(void *_Nonnull);
	void (*_Nonnull  renderWindow)(struct ag_window *_Nonnull);
	void (*_Nonnull  endRendering)(void *_Nonnull);
	void (*_Nonnull  fillRect)(void *_Nonnull, AG_Rect, AG_Color);
	void (*_Nullable updateRegion)(void *_Nonnull, AG_Rect);
	void (*_Nullable uploadTexture)(void *_Nonnull, Uint *_Nonnull,
					AG_Surface *_Nonnull, AG_TexCoord *_Nullable);
	int  (*_Nullable updateTexture)(void *_Nonnull, Uint,
	                                AG_Surface *_Nonnull, AG_TexCoord *_Nullable);
	void (*_Nullable deleteTexture)(void *_Nonnull, Uint);
	int  (*_Nullable setRefreshRate)(void *_Nonnull, int);

	void (*_Nonnull  pushClipRect)(void *_Nonnull, AG_Rect);
	void (*_Nonnull  popClipRect)(void *_Nonnull);
	void (*_Nonnull  pushBlendingMode)(void *_Nonnull, AG_AlphaFn, AG_AlphaFn);
	void (*_Nonnull  popBlendingMode)(void *_Nonnull);

	/* Hardware Cursor Operations */
	struct ag_cursor *_Nullable (*_Nonnull createCursor)
	                  (void *_Nonnull, Uint,Uint, const Uint8 *_Nonnull,
			   const Uint8 *_Nonnull, int,int);
	void (*_Nonnull freeCursor)(void *_Nonnull, struct ag_cursor *_Nonnull);
	int  (*_Nonnull setCursor)(void *_Nonnull, struct ag_cursor *_Nonnull);
	void (*_Nonnull unsetCursor)(void *_Nonnull);
	int  (*_Nonnull getCursorVisibility)(void *_Nonnull);
	void (*_Nonnull setCursorVisibility)(void *_Nonnull, int);

	/* Hardware Texture Operations */
	void (*_Nonnull blitSurface)(void *_Nonnull, struct ag_widget *_Nonnull,
	                             AG_Surface *_Nonnull, int,int);
	void (*_Nonnull blitSurfaceFrom)(void *_Nonnull, struct ag_widget *_Nonnull,
	                                 int, const AG_Rect *_Nullable, int,int);
	void (*_Nonnull blitSurfaceGL)(void *_Nonnull, struct ag_widget *_Nonnull,
	                               AG_Surface *_Nonnull, float,float);
	void (*_Nonnull blitSurfaceFromGL)(void *_Nonnull, struct ag_widget *_Nonnull,
	                                   int, float,float);
	void (*_Nonnull blitSurfaceFlippedGL)(void *_Nonnull,
	                                      struct ag_widget *_Nonnull,
	                                      int, float,float);
	void (*_Nullable backupSurfaces)(void *_Nonnull, struct ag_widget *_Nonnull);
	void (*_Nullable restoreSurfaces)(void *_Nonnull, struct ag_widget *_Nonnull);
	int  (*_Nullable renderToSurface)(void *_Nonnull, struct ag_widget *_Nonnull,
	                                  AG_Surface *_Nonnull *_Nullable);
	/* GUI Rendering Primitives */
	void (*_Nonnull putPixel)(void *_Nonnull, int,int, AG_Color);
	void (*_Nonnull putPixel32)(void *_Nonnull, int,int, Uint32);
	void (*_Nonnull putPixelRGB8)(void *_Nonnull, int,int, Uint8,Uint8,Uint8);
#if AG_MODEL == AG_LARGE
	void (*_Nonnull putPixel64)(void *_Nonnull, int,int, Uint64);
	void (*_Nonnull putPixelRGB16)(void *_Nonnull, int,int, Uint16,Uint16,Uint16);
#endif
	void (*_Nonnull blendPixel)(void *_Nonnull, int,int, AG_Color,
	                            AG_AlphaFn, AG_AlphaFn);
	void (*_Nonnull drawLine)(void *_Nonnull, int,int, int,int, AG_Color);
	void (*_Nonnull drawLineH)(void *_Nonnull, int,int, int, AG_Color);
	void (*_Nonnull drawLineV)(void *_Nonnull, int, int,int, AG_Color);
	void (*_Nonnull drawLineBlended)(void *_Nonnull, int,int, int,int,
	                                 AG_Color, AG_AlphaFn, AG_AlphaFn);
	void (*_Nonnull drawTriangle)(void *_Nonnull, AG_Pt,AG_Pt,AG_Pt, AG_Color);
	void (*_Nonnull drawArrow)(void *_Nonnull, Uint8, int,int, int, AG_Color);
	void (*_Nonnull drawBoxRounded)(void *_Nonnull, AG_Rect, int, int,
	                                AG_Color, AG_Color, AG_Color);
	void (*_Nonnull drawBoxRoundedTop)(void *_Nonnull, AG_Rect, int, int,
	                                   AG_Color, AG_Color, AG_Color);
	void (*_Nonnull drawCircle)(void *_Nonnull, int,int, int, AG_Color);
	void (*_Nonnull drawCircleFilled)(void *_Nonnull, int,int, int, AG_Color);
	void (*_Nonnull drawRectFilled)(void *_Nonnull, AG_Rect, AG_Color);
	void (*_Nonnull drawRectBlended)(void *_Nonnull, AG_Rect, AG_Color,
	                                 AG_AlphaFn, AG_AlphaFn);
	void (*_Nonnull drawRectDithered)(void *_Nonnull, AG_Rect, AG_Color);

	/* Font engine operations */
	void (*_Nonnull updateGlyph)(void *_Nonnull, struct ag_glyph *_Nonnull);
	void (*_Nonnull drawGlyph)(void *_Nonnull,
	                           const struct ag_glyph *_Nonnull, int,int);
	/* Display list management */
	void (*_Nullable deleteList)(void *_Nonnull, Uint);
} AG_DriverClass;

/* Generic driver instance. */
typedef struct ag_driver {
	struct ag_object _inherit;
	Uint id;                   	     /* Numerical instance ID */
	Uint flags;
#define AG_DRIVER_WINDOW_BG 0x02             /* Managed window background */

	AG_Surface *_Nonnull sRef;           /* Standard reference surface */
	AG_PixelFormat *_Nullable videoFmt;  /* Video pixel format (FB modes) */

	struct ag_keyboard *_Nullable kbd;   /* Primary keyboard device */
	struct ag_mouse    *_Nullable mouse; /* Primary mouse device */

	struct ag_cursor *_Nullable activeCursor;  /* Current cursor */
	AG_TAILQ_HEAD_(ag_cursor) cursors;         /* Available cursors */
	Uint                     nCursors;

	struct ag_glyph_cache *_Nonnull glyphCache; /* For text rendering */
	void *_Nullable gl;                         /* AG_GL_Context (GL modes) */
} AG_Driver;

/* Generic driver event (for custom event loops). */
enum ag_driver_event_type {
	AG_DRIVER_UNKNOWN,		/* Unknown event */
	AG_DRIVER_MOUSE_MOTION,		/* Cursor moved */
	AG_DRIVER_MOUSE_BUTTON_DOWN,	/* Mouse button pressed */
	AG_DRIVER_MOUSE_BUTTON_UP,	/* Mouse button released */
	AG_DRIVER_MOUSE_ENTER,		/* Mouse entering window (MW) */
	AG_DRIVER_MOUSE_LEAVE,		/* Mouse leaving window (MW) */
	AG_DRIVER_FOCUS_IN,		/* Focus on window (MW) */
	AG_DRIVER_FOCUS_OUT,		/* Focus out of window (MW) */
	AG_DRIVER_KEY_DOWN,		/* Key pressed */
	AG_DRIVER_KEY_UP,		/* Key released */
	AG_DRIVER_EXPOSE,		/* Video update needed */
	AG_DRIVER_VIDEORESIZE,		/* Video resize request */
	AG_DRIVER_CLOSE			/* Window close request */
};
typedef struct ag_driver_event {
	enum ag_driver_event_type type;	 /* Type of event */
	struct ag_window *_Nullable win; /* Associated window (AG_WM_MULTIPLE) */
	AG_TAILQ_ENTRY(ag_driver_event) events;
	union {
		struct {
			int x,y;		/* Cursor coordinates */
		} motion;
		struct {
			AG_MouseButton which;	/* Mouse button */
			int x,y;		/* Cursor coordinates */
		} button;
		struct {
			AG_KeySym ks;		/* Virtual key */
			Uint32 ucs;		/* Corresponding Unicode */
		} key;
		struct {
			int x,y, w,h;		/* Display offset, resolution */
		} videoresize;
	} data;
} AG_DriverEvent;

typedef AG_TAILQ_HEAD(ag_driver_eventq, ag_driver_event) AG_DriverEventQ;

#define AGDRIVER(obj)		((AG_Driver *)(obj))
#define AGDRIVER_CLASS(obj)	((struct ag_driver_class *)(AGOBJECT(obj)->cls))
#define AGDRIVER_SINGLE(drv)	(AGDRIVER_CLASS(drv)->wm == AG_WM_SINGLE)
#define AGDRIVER_MULTIPLE(drv)	(AGDRIVER_CLASS(drv)->wm == AG_WM_MULTIPLE)
#define AGDRIVER_BOUNDED_WIDTH(win,x) (((x) < 0) ? 0 : \
                                      ((x) > AGWIDGET(win)->w) ? (AGWIDGET(win)->w - 1) : (x))
#define AGDRIVER_BOUNDED_HEIGHT(win,y) (((y) < 0) ? 0 : \
                                       ((y) > AGWIDGET(win)->h) ? (AGWIDGET(win)->h - 1) : (y))
__BEGIN_DECLS
extern AG_ObjectClass agDriverClass;          /* Base AG_Driver class */
extern AG_Object agDrivers;                   /* Drivers VFS Root */
extern AG_DriverClass *_Nullable agDriverOps; /* Current driver class */
extern void *_Nonnull agDriverList[];         /* Available AG_DriverClass'es */
extern Uint           agDriverListSize;

#include <agar/config/have_clock_gettime.h>
#include <agar/config/have_pthreads.h>
#if defined(HAVE_CLOCK_GETTIME) && defined(HAVE_PTHREADS)
extern _Nonnull AG_Cond agCondBeginRender;	/* For agTimeOps_render */
extern _Nonnull AG_Cond agCondEndRender;
#endif

void AG_ListDriverNames(char *_Nonnull, AG_Size);

AG_Driver *_Nullable AG_DriverOpen(AG_DriverClass *_Nonnull);
void                 AG_DriverClose(AG_Driver *_Nonnull);

void AG_ViewCapture(void);

/*
 * Lookup a driver instance by ID.
 * The agDrivers VFS must be locked.
 */
static __inline__ AG_Driver *_Nullable _Pure_Attribute
AG_GetDriverByID(Uint id)
{
	AG_Driver *drv;

	AGOBJECT_FOREACH_CHILD(drv, &agDrivers, ag_driver) {
		if (drv->id == id)
			return (drv);
	}
	return (NULL);
}

/* Enter GUI rendering context. */
static __inline__ void
AG_BeginRendering(void *_Nonnull drv)
{
#if defined(HAVE_CLOCK_GETTIME) && defined(HAVE_PTHREADS)
	if (agTimeOps == &agTimeOps_renderer)		/* Renderer-aware ops */
		AG_CondBroadcast(&agCondBeginRender);
#endif
	agRenderingContext = 1;
	AGDRIVER_CLASS(drv)->beginRendering(drv);
}

/* Leave GUI rendering context. */
static __inline__ void
AG_EndRendering(void *_Nonnull drv)
{
	AGDRIVER_CLASS(drv)->endRendering(drv);
	agRenderingContext = 0;
#if defined(HAVE_CLOCK_GETTIME) && defined(HAVE_PTHREADS)
	if (agTimeOps == &agTimeOps_renderer)		/* Renderer-aware ops */
		AG_CondBroadcast(&agCondEndRender);
#endif
}
__END_DECLS

#include <agar/gui/drv_mw.h>
#include <agar/gui/drv_sw.h>

__BEGIN_DECLS
/* Return whether Agar is using OpenGL. */
static __inline__ int _Pure_Attribute
AG_UsingGL(void *_Nullable drv)
{
	if (drv != NULL) {
		return (AGDRIVER_CLASS(drv)->flags & AG_DRIVER_OPENGL);
	} else {
		return (agDriverOps->flags & AG_DRIVER_OPENGL);
	}
}

/* Return whether Agar is using SDL. */
static __inline__ int _Pure_Attribute
AG_UsingSDL(void *_Nullable drv)
{
	AG_DriverClass *dc = (drv != NULL) ? AGDRIVER_CLASS(drv) : agDriverOps;
	return (dc->flags & AG_DRIVER_SDL);
}

/* Query a driver for available display area in pixels. */
static __inline__ int
AG_GetDisplaySize(void *_Nullable drv, Uint *_Nonnull w, Uint *_Nonnull h)
{
	AG_DriverClass *dc = (drv != NULL) ? AGDRIVER_CLASS(drv) : agDriverOps;
	AG_DriverSw *dsw = (drv != NULL) ? (AG_DriverSw *)drv : agDriverSw;

	switch (dc->wm) {
	case AG_WM_SINGLE:
		*w = dsw->w;
		*h = dsw->h;
		return (0);
	case AG_WM_MULTIPLE:
		return dc->getDisplaySize(w, h);
	}
	return (-1);
}
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_DRV_H_ */
