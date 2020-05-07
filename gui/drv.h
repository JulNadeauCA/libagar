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
	struct ag_object_class _inherit;	/* [AG_Object] -> [AG_Driver] */

	const char *_Nonnull name;		/* Short name */
	enum ag_driver_type type;		/* Driver type */
	enum ag_driver_wm_type wm;		/* Window manager type */
#ifdef AG_HAVE_64BIT
	Uint64 flags;				/* Capabilities */
#else
	Uint flags;
#endif
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
	void (*_Nonnull  fillRect)(void *_Nonnull, const AG_Rect *_Nonnull,
	                           const AG_Color *_Nonnull);
	void (*_Nullable updateRegion)(void *_Nonnull, const AG_Rect *_Nonnull);

	void (*_Nullable uploadTexture)(void *_Nonnull, Uint *_Nonnull,
	                                AG_Surface *_Nonnull,
	                                AG_TexCoord *_Nullable);
	void (*_Nonnull  updateTexture)(void *_Nonnull, Uint,
	                                AG_Surface *_Nonnull,
	                                AG_TexCoord *_Nullable);
	void (*_Nonnull  deleteTexture)(void *_Nonnull, Uint);

	int  (*_Nullable setRefreshRate)(void *_Nonnull, int);

	void (*_Nonnull  pushClipRect)(void *_Nonnull, const AG_Rect *_Nonnull);
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
#ifdef HAVE_OPENGL
	void (*_Nonnull blitSurfaceGL)(void *_Nonnull, struct ag_widget *_Nonnull,
	                               AG_Surface *_Nonnull, float,float);
	void (*_Nonnull blitSurfaceFromGL)(void *_Nonnull, struct ag_widget *_Nonnull,
	                                   int, float,float);
	void (*_Nonnull blitSurfaceFlippedGL)(void *_Nonnull,
	                                      struct ag_widget *_Nonnull,
	                                      int, float,float);
#endif
	void (*_Nullable backupSurfaces)(void *_Nonnull, struct ag_widget *_Nonnull);
	void (*_Nullable restoreSurfaces)(void *_Nonnull, struct ag_widget *_Nonnull);
	int  (*_Nullable renderToSurface)(void *_Nonnull, struct ag_widget *_Nonnull,
	                                  AG_Surface *_Nonnull *_Nullable);
	/* GUI Rendering Primitives */
	void (*_Nonnull putPixel)(void *_Nonnull, int,int,
	                          const AG_Color *_Nonnull);
	void (*_Nonnull putPixel32)(void *_Nonnull, int,int, Uint32);
	void (*_Nonnull putPixelRGB8)(void *_Nonnull, int,int, Uint8,Uint8,Uint8);
#if AG_MODEL == AG_LARGE
	void (*_Nonnull putPixel64)(void *_Nonnull, int,int, Uint64);
	void (*_Nonnull putPixelRGB16)(void *_Nonnull, int,int, Uint16,Uint16,Uint16);
#endif
	void (*_Nonnull blendPixel)(void *_Nonnull, int,int,
	                            const AG_Color *_Nonnull,
	                            AG_AlphaFn, AG_AlphaFn);
	void (*_Nonnull drawLine)(void *_Nonnull, int,int, int,int,
	                          const AG_Color *_Nonnull);
	void (*_Nonnull drawLineH)(void *_Nonnull, int,int, int,
	                           const AG_Color *_Nonnull);
	void (*_Nonnull drawLineV)(void *_Nonnull, int, int,int,
	                           const AG_Color *_Nonnull);
	void (*_Nonnull drawLineBlended)(void *_Nonnull, int,int, int,int,
	                                 const AG_Color *_Nonnull,
	                                 AG_AlphaFn, AG_AlphaFn);
	void (*_Nonnull drawLineW)(void *_Nonnull, int,int, int,int,
	                           const AG_Color *_Nonnull, float);
	void (*_Nonnull drawLineW_Sti16)(void *_Nonnull, int,int, int,int,
	                                 const AG_Color *_Nonnull, float, Uint16);

	void (*_Nonnull drawTriangle)(void *_Nonnull, const AG_Pt *_Nonnull,
	                              const AG_Pt *_Nonnull, const AG_Pt *_Nonnull,
	                              const AG_Color *_Nonnull);
	void (*_Nonnull drawPolygon)(void *_Nonnull, const AG_Pt *_Nonnull, Uint,
	                             const AG_Color *_Nonnull);
	void (*_Nonnull drawPolygonSti32)(void *_Nonnull, const AG_Pt *_Nonnull,
	                                  Uint, const AG_Color *_Nonnull,
	                                  const Uint8 *_Nonnull);

	void (*_Nonnull drawArrow)(void *_Nonnull, Uint8, int,int, int,
	                           const AG_Color *_Nonnull);
	void (*_Nonnull drawBoxRounded)(void *_Nonnull, const AG_Rect *_Nonnull,
	                                int, int, const AG_Color *_Nonnull,
					const AG_Color *_Nonnull,
					const AG_Color *_Nonnull);
	void (*_Nonnull drawBoxRoundedTop)(void *_Nonnull, const AG_Rect *_Nonnull,
	                                   int, int, const AG_Color *_Nonnull,
	                                   const AG_Color *_Nonnull,
	                                   const AG_Color *_Nonnull);
	void (*_Nonnull drawCircle)(void *_Nonnull, int,int, int,
	                            const AG_Color *_Nonnull);
	void (*_Nonnull drawCircleFilled)(void *_Nonnull, int,int, int,
	                                  const AG_Color *_Nonnull);
	void (*_Nonnull drawRectFilled)(void *_Nonnull, const AG_Rect *_Nonnull,
	                                const AG_Color *_Nonnull);
	void (*_Nonnull drawRectBlended)(void *_Nonnull, const AG_Rect *_Nonnull,
	                                 const AG_Color *_Nonnull,
	                                 AG_AlphaFn, AG_AlphaFn);
	void (*_Nonnull drawRectDithered)(void *_Nonnull, const AG_Rect *_Nonnull,
	                                  const AG_Color *_Nonnull);

	/* Typography */
	void (*_Nonnull updateGlyph)(void *_Nonnull, struct ag_glyph *_Nonnull);
	void (*_Nonnull drawGlyph)(void *_Nonnull, const struct ag_glyph *_Nonnull,
	                           int,int);

	/* Display list management */
	void (*_Nullable deleteList)(void *_Nonnull, Uint);

	/* Clipboard access */
	char *_Nullable (*_Nullable getClipboardText)(void *_Nonnull);
	int             (*_Nullable setClipboardText)(void *_Nonnull,
	                                              const char *_Nonnull);
} AG_DriverClass;

/* Generic driver instance. */
typedef struct ag_driver {
	struct ag_object _inherit;		/* AG_Object -> AG_Driver */
	Uint id;				/* Numerical instance ID */
	Uint flags;
#define AG_DRIVER_WINDOW_BG 0x02		/* Managed window background */

	AG_Surface *_Nonnull sRef;		/* Standard reference surface */
	AG_PixelFormat *_Nullable videoFmt;	/* Video pixel format (FB modes) */

	struct ag_keyboard *_Nullable kbd;	/* Primary keyboard device */
	struct ag_mouse    *_Nullable mouse;	/* Primary mouse device */
	
	struct ag_glyph_cache *_Nonnull glyphCache; /* For text rendering */
	void *_Nullable gl;                         /* AG_GL_Context (GL modes) */

	struct ag_cursor *_Nullable activeCursor; /* Current cursor */
	AG_TAILQ_HEAD_(ag_cursor) cursors;	  /* Available cursors */
	Uint                     nCursors;

	Uint32 tRender;                           /* Rendering start (ticks) */
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
	Uint32 _pad;
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

#define AGDRIVER(obj)            ((AG_Driver *)(obj))
#define AGCDRIVER(obj)           ((const AG_Driver *)(obj))
#define AG_DRIVER_SELF()          AGDRIVER( AG_OBJECT(0,"AG_Driver:*") )
#define AG_DRIVER_PTR(n)          AGDRIVER( AG_OBJECT((n),"AG_Driver:*") )
#define AG_DRIVER_NAMED(n)        AGDRIVER( AG_OBJECT_NAMED((n),"AG_Driver:*") )
#define AG_CONST_DRIVER_SELF()   AGCDRIVER( AG_CONST_OBJECT(0,"AG_Driver:*") )
#define AG_CONST_DRIVER_PTR(n)   AGCDRIVER( AG_CONST_OBJECT((n),"AG_Driver:*") )
#define AG_CONST_DRIVER_NAMED(n) AGCDRIVER( AG_CONST_OBJECT_NAMED((n),"AG_Driver:*") )

#define AGDRIVER_CLASS(obj)	((struct ag_driver_class *)(AGOBJECT(obj)->cls))

#ifdef AG_TYPE_SAFETY
# define AGDRIVER_SINGLE(drv)	(AG_OBJECT_VALID(drv) && (AGDRIVER_CLASS(drv)->wm == AG_WM_SINGLE))
# define AGDRIVER_MULTIPLE(drv)	(AG_OBJECT_VALID(drv) && (AGDRIVER_CLASS(drv)->wm == AG_WM_MULTIPLE))
#else
# define AGDRIVER_SINGLE(drv)	(AGDRIVER_CLASS(drv)->wm == AG_WM_SINGLE)
# define AGDRIVER_MULTIPLE(drv)	(AGDRIVER_CLASS(drv)->wm == AG_WM_MULTIPLE)
#endif

#define AGDRIVER_BOUNDED_WIDTH(win,x) (((x) < 0) ? 0 : \
                                      ((x) > AGWIDGET(win)->w) ? (AGWIDGET(win)->w - 1) : (x))
#define AGDRIVER_BOUNDED_HEIGHT(win,y) (((y) < 0) ? 0 : \
                                       ((y) > AGWIDGET(win)->h) ? (AGWIDGET(win)->h - 1) : (y))
__BEGIN_DECLS
extern AG_ObjectClass            agDriverClass;	/* Base AG_Driver class */
extern AG_Object                 agDrivers;	/* Drivers VFS Root */
extern AG_DriverClass *_Nullable agDriverOps;	/* Current driver class */

extern AG_DriverClass *_Nonnull agDriverList[];	/* Available driver classes */

extern const char *_Nonnull agDriverTypeNames[];
extern const char *_Nonnull agDriverWmTypeNames[];

#include <agar/config/have_clock_gettime.h>
#include <agar/config/have_pthreads.h>
#if defined(HAVE_CLOCK_GETTIME) && defined(HAVE_PTHREADS)
extern _Nonnull_Cond AG_Cond agCondBeginRender;	/* For agTimeOps_render */
extern _Nonnull_Cond AG_Cond agCondEndRender;
#endif

void AG_ListDriverNames(char *_Nonnull, AG_Size);

AG_Driver *_Nullable AG_DriverOpen(AG_DriverClass *_Nonnull);
void                 AG_DriverClose(AG_Driver *_Nonnull);
AG_Driver *_Nullable AG_GetDriverByID(Uint) _Pure_Attribute;

void AG_BeginRendering(void *_Nonnull);
void AG_EndRendering(void *_Nonnull);

void AG_ViewCapture(void);

int AG_UsingGL(void *_Nullable) _Pure_Attribute;
int AG_UsingSDL(void *_Nullable) _Pure_Attribute;
int AG_GetDisplaySize(void *_Nullable, Uint *_Nonnull,Uint *_Nonnull);
__END_DECLS

#include <agar/gui/drv_mw.h>
#include <agar/gui/drv_sw.h>

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_DRV_H_ */
