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
	const char *name;			/* Short name */
	enum ag_driver_type type;		/* Driver type */
	enum ag_driver_wm_type wm;		/* Window manager type */
	Uint flags;
#define AG_DRIVER_OPENGL	0x01		/* Supports OpenGL calls */
#define AG_DRIVER_SDL		0x02		/* Supports SDL calls */
#define AG_DRIVER_TEXTURES	0x04		/* Support texture ops */

	/* Initialization */
	int  (*open)(void *drv, const char *spec);
	void (*close)(void *drv);
	int  (*getDisplaySize)(Uint *w, Uint *h);
	/* Event processing */
	void (*beginEventProcessing)(void *drv);
	int  (*pendingEvents)(void *drv);
	int  (*getNextEvent)(void *drv, struct ag_driver_event *dev);
	int  (*processEvent)(void *drv, struct ag_driver_event *dev);
	void (*genericEventLoop)(void *drv);
	void (*endEventProcessing)(void *drv);
	void (*terminate)(void);
	/* GUI rendering */
	void (*beginRendering)(void *drv);
	void (*renderWindow)(struct ag_window *);
	void (*endRendering)(void *drv);
	/* Primitives */
	void (*fillRect)(void *drv, AG_Rect r, AG_Color c);
	/* Update video region (rendering context; FB driver specific) */
	void (*updateRegion)(void *drv, AG_Rect r);
	/* Texture operations (GL driver specific) */
	void (*uploadTexture)(void *drv, Uint *, AG_Surface *, AG_TexCoord *);
	int  (*updateTexture)(void *drv, Uint, AG_Surface *, AG_TexCoord *);
	void (*deleteTexture)(void *drv, Uint);
	/* Request a specific refresh rate (driver specific) */
	int (*setRefreshRate)(void *drv, int fps);
	/* Clipping and blending control (rendering context) */
	void (*pushClipRect)(void *drv, AG_Rect r);
	void (*popClipRect)(void *drv);
	void (*pushBlendingMode)(void *drv, AG_BlendFn srcFn, AG_BlendFn dstFn);
	void (*popBlendingMode)(void *drv);
	/* Hardware cursor operations */
	struct ag_cursor *(*createCursor)(void *drv, Uint w, Uint h, const Uint8 *data, const Uint8 *mask, int xHot, int yHot);
	void (*freeCursor)(void *drv, struct ag_cursor *curs);
	int  (*setCursor)(void *drv, struct ag_cursor *curs);
	void (*unsetCursor)(void *drv);
	int  (*getCursorVisibility)(void *drv);
	void (*setCursorVisibility)(void *drv, int flag);
	/* Widget surface operations (rendering context) */
	void (*blitSurface)(void *drv, struct ag_widget *wid, AG_Surface *s, int x, int y);
	void (*blitSurfaceFrom)(void *drv, struct ag_widget *wid, struct ag_widget *widSrc, int s, AG_Rect *r, int x, int y);
	void (*blitSurfaceGL)(void *drv, struct ag_widget *wid, AG_Surface *s, float w, float h);
	void (*blitSurfaceFromGL)(void *drv, struct ag_widget *wid, int s, float w, float h);
	void (*blitSurfaceFlippedGL)(void *drv, struct ag_widget *wid, int s, float w, float h);
	void (*backupSurfaces)(void *drv, struct ag_widget *wid);
	void (*restoreSurfaces)(void *drv, struct ag_widget *wid);
	int  (*renderToSurface)(void *drv, struct ag_widget *wid, AG_Surface **su);
	/* Rendering operations (rendering context) */
	void (*putPixel)(void *drv, int x, int y, AG_Color c);
	void (*putPixel32)(void *drv, int x, int y, Uint32 c);
	void (*putPixelRGB)(void *drv, int x, int y, Uint8 r, Uint8 g, Uint8 b);
	void (*blendPixel)(void *drv, int x, int y, AG_Color c, AG_BlendFn fnSrc, AG_BlendFn fnDst);
	void (*drawLine)(void *drv, int x1, int y1, int x2, int y2, AG_Color C);
	void (*drawLineH)(void *drv, int x1, int x2, int y, AG_Color C);
	void (*drawLineV)(void *drv, int x, int y1, int y2, AG_Color C);
	void (*drawLineBlended)(void *drv, int x1, int y1, int x2, int y2, AG_Color C, AG_BlendFn fnSrc, AG_BlendFn fnDst);
	void (*drawArrowUp)(void *drv, int x0, int y0, int h, AG_Color C[2]);
	void (*drawArrowDown)(void *drv, int x0, int y0, int h, AG_Color C[2]);
	void (*drawArrowLeft)(void *drv, int x0, int y0, int h, AG_Color C[2]);
	void (*drawArrowRight)(void *drv, int x0, int y0, int h, AG_Color C[2]);
	void (*drawBoxRounded)(void *drv, AG_Rect r, int z, int rad, AG_Color C[3]);
	void (*drawBoxRoundedTop)(void *drv, AG_Rect r, int z, int rad, AG_Color C[3]);
	void (*drawCircle)(void *drv, int x, int y, int r, AG_Color C);
	void (*drawCircleFilled)(void *drv, int x, int y, int r, AG_Color C);
	void (*drawRectFilled)(void *drv, AG_Rect r, AG_Color C);
	void (*drawRectBlended)(void *drv, AG_Rect r, AG_Color C, AG_BlendFn fnSrc, AG_BlendFn fnDst);
	void (*drawRectDithered)(void *drv, AG_Rect r, AG_Color C);
	void (*updateGlyph)(void *drv, struct ag_glyph *);
	void (*drawGlyph)(void *drv, const struct ag_glyph *, int x, int y);
	/* Display list management (GL driver specific) */
	void (*deleteList)(void *drv, Uint);
} AG_DriverClass;

/* Generic driver instance. */
typedef struct ag_driver {
	struct ag_object _inherit;
	Uint id;			/* Numerical instance ID */
	Uint flags;
#define AG_DRIVER_WINDOW_BG	0x02	/* Managed window background */

	AG_Surface *sRef;		/* "Reference" surface */
	AG_PixelFormat *videoFmt;	/* Video pixel format (for
					   packed-pixel FB modes) */
	struct ag_keyboard *kbd;	/* Main keyboard device */
	struct ag_mouse *mouse;		/* Main mouse device */
	struct ag_cursor *activeCursor;	/* Effective cursor */
	AG_TAILQ_HEAD_(ag_cursor) cursors; /* Registered cursors */
	Uint                     nCursors;
	struct ag_glyph_cache *glyphCache; /* Cache of rendered glyphs */
	void *gl;			/* AG_GL_Context (for GL drivers) */
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
	enum ag_driver_event_type type;	/* Type of event */
	struct ag_window *win;		/* Associated window (AG_WM_MULTIPLE) */
	union {
		struct {
			int x, y;
		} motion;
		struct {
			AG_MouseButton which;
			int x, y;
		} button;
		struct {
			AG_KeySym ks;
			Uint32 ucs;
		} key;
		struct {
			int x, y, w, h;
		} videoresize;
	} data;
	AG_TAILQ_ENTRY(ag_driver_event) events;
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
extern AG_ObjectClass agDriverClass;

extern AG_Object       agDrivers;	/* Drivers VFS */
extern AG_DriverClass *agDriverOps;	/* Current driver class */
extern void           *agDriverList[];	/* Available drivers (AG_DriverClass) */
extern Uint            agDriverListSize;

#include <agar/config/have_clock_gettime.h>
#include <agar/config/have_pthreads.h>
#if defined(HAVE_CLOCK_GETTIME) && defined(HAVE_PTHREADS)
extern AG_Cond agCondBeginRender;	/* For agTimeOps_render */
extern AG_Cond agCondEndRender;
#endif

void       AG_ListDriverNames(char *, size_t)
                              BOUNDED_ATTRIBUTE(__string__, 1, 2);
int        AG_DriverProbe(AG_DriverClass *, const char *);
AG_Driver *AG_DriverOpen(AG_DriverClass *);
void       AG_DriverClose(AG_Driver *);
void       AG_ViewCapture(void);

/* Lookup a driver instance by ID */
static __inline__ AG_Driver *
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
AG_BeginRendering(void *drv)
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
AG_EndRendering(void *drv)
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
static __inline__ int
AG_UsingGL(void *drv)
{
	if (drv != NULL) {
		return (AGDRIVER_CLASS(drv)->flags & AG_DRIVER_OPENGL);
	} else {
		return (agDriverOps->flags & AG_DRIVER_OPENGL);
	}
}

/* Return whether Agar is using SDL. */
static __inline__ int
AG_UsingSDL(void *drv)
{
	AG_DriverClass *dc = (drv != NULL) ? AGDRIVER_CLASS(drv) : agDriverOps;
	return (dc->flags & AG_DRIVER_SDL);
}

/* Query a driver for available display area in pixels. */
static __inline__ int
AG_GetDisplaySize(void *drv, Uint *w, Uint *h)
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
