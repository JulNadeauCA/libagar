/*	Public domain	*/

#ifndef _AGAR_WIDGET_H_
#define _AGAR_WIDGET_H_

#include <agar/config/have_opengl.h>

#include <agar/gui/colors.h>
#include <agar/gui/geometry.h>
#include <agar/gui/surface.h>
#include <agar/gui/view.h>
#include <agar/gui/style.h>

#include <agar/gui/mouse.h>
#include <agar/gui/kbd.h>

#include <agar/gui/begin.h>

#define AG_SIZE_SPEC_MAX		256
#define AG_WIDGET_BINDING_NAME_MAX	16

typedef struct ag_size_req {
	int w, h;			/* Requested geometry in pixels */
} AG_SizeReq;

typedef struct ag_size_alloc {
	int w, h;			/* Allocated geometry in pixels */
	int x, y;			/* Allocated position in pixels */
} AG_SizeAlloc;

typedef struct ag_widget_class {
	struct ag_object_class _inherit;
	void (*draw)(void *);
	void (*size_request)(void *, AG_SizeReq *);
	int  (*size_allocate)(void *, const AG_SizeAlloc *);
} AG_WidgetClass;

typedef enum ag_widget_sizespec {
	AG_WIDGET_BAD_SPEC,	/* Parser error */
	AG_WIDGET_PIXELS,	/* Pixel count */
	AG_WIDGET_PERCENT,	/* % of available space */
	AG_WIDGET_STRINGLEN,	/* Width of given string */
	AG_WIDGET_FILL		/* Fill remaining space */
} AG_SizeSpec;

enum ag_widget_packing {
	AG_PACK_HORIZ,
	AG_PACK_VERT
};

typedef struct ag_flag_descr {
	Uint bitmask;			/* Bitmask */
	const char *descr;		/* Bit(s) description */
	int writeable;			/* User-editable */
} AG_FlagDescr;

struct ag_popup_menu;

typedef enum ag_action_type {
	AG_ACTION_FN,			/* Execute function */
	AG_ACTION_SET_INT,		/* Set an integer */
	AG_ACTION_TOGGLE_INT,		/* Toggle an integer */
	AG_ACTION_SET_FLAG,		/* Set specified bits in a word */
	AG_ACTION_TOGGLE_FLAG		/* Toggle specified bits in a word */
} AG_ActionType;

typedef struct ag_action {
	AG_ActionType type;
	struct ag_widget *widget;	/* Back pointer to widget */
	AG_Event *fn;			/* Callback function */
	void *p;			/* Target (for SET_*) */
	int val;			/* Value (for SET_*) */
	Uint bitmask;			/* Bitmask (for SET_FLAG) */
} AG_Action;

typedef enum ag_action_event_type {
	AG_ACTION_ON_BUTTONDOWN,	/* On mouse-button-down */
	AG_ACTION_ON_BUTTONUP,		/* On mouse-button-up */
	AG_ACTION_ON_KEYDOWN,		/* On key-down */
	AG_ACTION_ON_KEYUP		/* On key-up */
#define AG_ACTION_ON_BUTTON \
	AG_ACTION_ON_BUTTONDOWN		/* For mousewheel events */
} AG_ActionEventType;

typedef struct ag_action_tie {
	AG_ActionEventType type;		/* Trigger event type */
	union {
		AG_MouseButton button;		/* Button index */
		struct {
			AG_KeySym sym;		/* Matching symbol */
			AG_KeyMod mod;		/* Matching modifier */
		} key;
	} data;
	char action[64];			/* Action name */
} AG_ActionTie;

typedef struct ag_widget {
	struct ag_object obj;

	Uint flags;
#define AG_WIDGET_FOCUSABLE		0x000001 /* Can grab focus */
#define AG_WIDGET_FOCUSED		0x000002 /* Holds focus (optimization) */
#define AG_WIDGET_UNFOCUSED_MOTION	0x000004 /* All mousemotion events */
#define AG_WIDGET_UNFOCUSED_BUTTONUP	0x000008 /* All mousebuttonup events */
#define AG_WIDGET_UNFOCUSED_BUTTONDOWN	0x000010 /* All mousebuttondown events */
#define AG_WIDGET_HFILL			0x000040 /* Expand to fill width */
#define AG_WIDGET_VFILL			0x000080 /* Expand to fill height */
#define AG_WIDGET_HIDE			0x000200 /* Don't draw this widget */
#define AG_WIDGET_DISABLED		0x000400 /* Don't respond to input */
#define AG_WIDGET_CATCH_TAB		0x001000 /* Catch tab key events */
#define AG_WIDGET_PRIO_MOTION		0x002000 /* Block mousemotion events to any other widget, regardless of focus */
#define AG_WIDGET_UNDERSIZE		0x004000 /* Size allocation failed */
#define AG_WIDGET_NOSPACING		0x008000 /* Disable spacings around widget; container-specific */
#define AG_WIDGET_UNFOCUSED_KEYDOWN	0x010000 /* All mousebuttondown events */
#define AG_WIDGET_UNFOCUSED_KEYUP	0x020000 /* All mousebuttondown events */
#define AG_WIDGET_DEBUG_RSENS		0x040000 /* Debug sensitivity rect */
#define AG_WIDGET_TABLE_EMBEDDABLE	0x080000 /* Usable in polled tables */
#define AG_WIDGET_UPDATE_WINDOW		0x100000 /* Request an AG_WindowUpdate() as soon as possible */
#define AG_WIDGET_EXPAND		(AG_WIDGET_HFILL|AG_WIDGET_VFILL)

	int x, y;			/* Coordinates in container */
	int w, h;			/* Allocated geometry */
	AG_Rect2 rView;			/* Computed view coordinates */
	AG_Rect2 rSens;			/* Rectangle of sensitivity (i.e., to cursor events), in view coords */
	AG_Style *style;		/* Style (inherited from parent) */

	AG_Surface **surfaces;		/* Registered surfaces */
	Uint *surfaceFlags;		/* Surface flags */
#define AG_WIDGET_SURFACE_NODUP	0x01	/* Don't free on destroy */
#define AG_WIDGET_SURFACE_REGEN	0x02	/* Texture needs to be regenerated */
	Uint nsurfaces;

	/* For OpenGL */
	Uint *textures;			/* Cached OpenGL textures */
	float *texcoords;		/* Cached texture coordinates */
	Uint *textureGC;		/* Textures queued for deletion */
	Uint nTextureGC;

	AG_Mutex bindings_lock;		 	/* Lock on bindings */
	AG_SLIST_HEAD(,ag_popup_menu) menus;	/* Managed menus */
	struct ag_widget *focusFwd;		/* For ForwardFocus() */
	struct ag_window *window;		/* Back ptr to parent window */

	AG_Tbl        actions;			/* Registered actions */
	AG_ActionTie *mouseActions;		/* Mouse event ties */
	Uint         nMouseActions;
	AG_ActionTie *keyActions;		/* Keyboard event ties */
	Uint         nKeyActions;
} AG_Widget;

#define AGWIDGET(wi)		((AG_Widget *)(wi))
#define AGWIDGET_OPS(wi)	((AG_WidgetClass *)AGOBJECT(wi)->cls)
#define AGWIDGET_SUPER_OPS(wi)	((AG_WidgetClass *)AGOBJECT(wi)->cls->super)

#define AGWIDGET_SURFACE(wi, ind)	AGWIDGET(wi)->surfaces[ind]
#define AGWIDGET_TEXTURE(wi, ind)	AGWIDGET(wi)->textures[ind]
#define AGWIDGET_TEXCOORD(wi, ind)	AGWIDGET(wi)->texcoords[(ind)*4]
#define AGWIDGET_SURFACE_NODUP(wi, ind)	(AGWIDGET(wi)->surfaceFlags[ind] & \
					 AG_WIDGET_SURFACE_NODUP)
#define AGSTYLE(p)			 AGWIDGET(p)->style

#define AGWIDGET_FOREACH_CHILD(var, ob) \
	AGOBJECT_FOREACH_CHILD(var, ob, ag_widget)
#define AGWIDGET_NEXT_CHILD(var) \
	AGOBJECT_NEXT_CHILD((var),ag_widget)

#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_GUI)
#define WIDGET(wi)			AGWIDGET(wi)
#define WIDGET_OPS(wi)			AGWIDGET_OPS(wi)
#define WIDGET_SUPER_OPS(wi)		AGWIDGET_SUPER_OPS(wi)
#define WSURFACE(wi,ind)		AGWIDGET_SURFACE((wi),(ind))
#define WTEXTURE(wi,ind)		AGWIDGET_TEXTURE((wi),(ind))
#define WTEXCOORD(wi,ind)		AGWIDGET_TEXCOORD((wi),(ind))
#define WSURFACE_NODUP(wi,ind)		AGWIDGET_SURFACE_NODUP((wi),(ind))
#define STYLE(p)                        AGSTYLE(p)
#define WIDTH(p)			AGWIDGET(p)->w
#define HEIGHT(p)			AGWIDGET(p)->h
#define WIDGET_FOREACH_CHILD(var,ob)	AGWIDGET_FOREACH_CHILD(var,ob)
#define WIDGET_NEXT_CHILD(var)		AGWIDGET_NEXT_CHILD(var)
#endif

struct ag_window;

__BEGIN_DECLS
extern AG_WidgetClass agWidgetClass;

void       AG_WidgetDraw(void *);
void       AG_WidgetSizeReq(void *, AG_SizeReq *);
int        AG_WidgetSizeAlloc(void *, AG_SizeAlloc *);
void       AG_WidgetSetFocusable(void *, int);
void       AG_WidgetForwardFocus(void *, void *);

int        AG_WidgetFocus(void *);
void       AG_WidgetUnfocus(void *);
AG_Widget *AG_WidgetFindFocused(void *);
void      *AG_WidgetFindPoint(const char *, int, int);
void      *AG_WidgetFindRect(const char *, int, int, int, int);
void       AG_WidgetUpdateCoords(void *, int, int);

int	 AG_WidgetMapSurface(void *, AG_Surface *);
int	 AG_WidgetMapSurfaceNODUP(void *, AG_Surface *);
void	 AG_WidgetReplaceSurface(void *, int, AG_Surface *);
void	 AG_WidgetReplaceSurfaceNODUP(void *, int, AG_Surface *);
#define	 AG_WidgetUnmapSurface(w, n) \
	 AG_WidgetReplaceSurface((w),(n),NULL)
void	 AG_WidgetBlit(void *, AG_Surface *, int, int);
void	 AG_WidgetBlitFrom(void *, void *, int, AG_Rect *, int, int);
#define  AG_WidgetBlitSurface(p,n,x,y) \
	 AG_WidgetBlitFrom((p),(p),(n),NULL,(x),(y))
#ifdef HAVE_OPENGL
void	 AG_WidgetBlitGL(void *, AG_Surface *, float, float);
void	 AG_WidgetBlitSurfaceGL(void *, int, float, float);
void	 AG_WidgetBlitSurfaceFlippedGL(void *, int, float, float);
void	 AG_WidgetPutPixel32_GL(void *, int, int, Uint32);
void	 AG_WidgetPutPixelRGB_GL(void *, int, int, Uint8, Uint8, Uint8);
void	 AG_WidgetFreeResourcesGL(AG_Widget *);
void	 AG_WidgetRegenResourcesGL(AG_Widget *);
#endif
void	 AG_PushClipRect(void *, AG_Rect);
void	 AG_PopClipRect(void);
void	 AG_SetCursor(int);
void	 AG_UnsetCursor(void);
#define	 AG_WidgetPutPixel AG_WidgetPutPixel32
#define	 AG_WidgetBlendPixel AG_WidgetBlendPixelRGBA
void	 AG_WidgetBlendPixelRGBA(void *, int, int, Uint8 [4], enum ag_blend_func);

int      AG_WidgetSensitive(void *, int, int);
void     AG_WidgetMouseMotion(struct ag_window *, AG_Widget *, int, int, int, int, int);
void     AG_WidgetMouseButtonUp(struct ag_window *, AG_Widget *, int, int, int);
int      AG_WidgetMouseButtonDown(struct ag_window *, AG_Widget *, int, int, int);
void     AG_WidgetUnfocusedKeyUp(AG_Widget *, int, int, int);
void     AG_WidgetUnfocusedKeyDown(AG_Widget *, int, int, int);

AG_SizeSpec AG_WidgetParseSizeSpec(const char *, int *);
int         AG_WidgetScrollDelta(Uint32 *);
void       *AG_WidgetFind(AG_Display *, const char *);
void        AG_WidgetShownRecursive(void *);
void        AG_WidgetHiddenRecursive(void *);

void        AG_WidgetInheritDraw(void *);
void        AG_WidgetInheritSizeRequest(void *, AG_SizeReq *);
int         AG_WidgetInheritSizeAllocate(void *, const AG_SizeAlloc *);
AG_Surface *AG_WidgetSurface(void *);

AG_Action  *AG_ActionFn(void *, const char *, AG_EventFn, const char *, ...);
AG_Action  *AG_ActionSetInt(void *, const char *, int *, int);
AG_Action  *AG_ActionToggleInt(void *, const char *, int *);
AG_Action  *AG_ActionSetFlag(void *, const char *, Uint *, Uint, int);
AG_Action  *AG_ActionToggleFlag(void *, const char *, Uint *, Uint);

void        AG_ActionOnButtonDown(void *, int, const char *);
void        AG_ActionOnButtonUp(void *, int, const char *);
#define     AG_ActionOnButton(w,b,a) \
            AG_ActionOnButtonDown((w),(b),(a))

void        AG_ActionOnKeyDown(void *, AG_KeySym, AG_KeyMod, const char *);
void        AG_ActionOnKeyUp(void *, AG_KeySym, AG_KeyMod, const char *);

int         AG_ExecMouseAction(void *, AG_ActionEventType, int, int, int);
int         AG_ExecKeyAction(void *, AG_ActionEventType, AG_KeySym, AG_KeyMod);
int         AG_ExecAction(void *, AG_Action *);

/* Set the widget state. */
static __inline__ void
AG_WidgetEnable(void *p)
{
	AG_ObjectLock(p);
	if (AGWIDGET(p)->flags & AG_WIDGET_DISABLED) {
		AGWIDGET(p)->flags &= ~(AG_WIDGET_DISABLED);
		AG_PostEvent(NULL, p, "widget-enabled", NULL);
	}
	AG_ObjectUnlock(p);
}
static __inline__ void
AG_WidgetDisable(void *p)
{
	AG_ObjectLock(p);
	if (!(AGWIDGET(p)->flags & AG_WIDGET_DISABLED)) {
		AGWIDGET(p)->flags |= AG_WIDGET_DISABLED;
		AG_PostEvent(NULL, p, "widget-disabled", NULL);
	}
	AG_ObjectUnlock(p);
}

/* Return the widget state. The Widget object must be locked. */
static __inline__ int
AG_WidgetEnabled(void *p)
{
	return !(AGWIDGET(p)->flags & AG_WIDGET_DISABLED);
}
static __inline__ int
AG_WidgetDisabled(void *p)
{
	return (AGWIDGET(p)->flags & AG_WIDGET_DISABLED);
}

/*
 * Return the focus state of the widget inside of its parent window (not
 * necessarily the effective focus). The Widget object must be locked.
 */
static __inline__ int
AG_WidgetIsFocusedInWindow(void *p)
{
	return (AGWIDGET(p)->flags & AG_WIDGET_FOCUSED);
}

/* Test whether view coordinates x,y lie in widget's allocated space. */
static __inline__ int
AG_WidgetArea(void *p, int x, int y)
{
	AG_Widget *wid = AGWIDGET(p);

	return (x > wid->rView.x1 && y > wid->rView.y1 &&
	        x < wid->rView.x2 && y < wid->rView.y2);
}

/* Test whether widget coordinates x,y lie in widget's allocated space. */
static __inline__ int
AG_WidgetRelativeArea(void *p, int x, int y)
{
	AG_Widget *wid = AGWIDGET(p);

	return (x >= 0 &&
	        y >= 0 &&
	        x < wid->w &&
		y < wid->h);
}

/* Write a single pixel at widget-relative coordinates (32-bit agVideoFmt). */
static __inline__ void
AG_WidgetPutPixel32(void *p, int wx, int wy, Uint32 color)
{
	AG_Widget *wid = AGWIDGET(p);
	int vx = wid->rView.x1 + wx;
	int vy = wid->rView.y1 + wy;

#ifdef HAVE_OPENGL
	if (agView->opengl) {
		AG_WidgetPutPixel32_GL(p, vx,vy, color);
	} else
#endif
	if (!AG_CLIPPED_PIXEL(agView->v, vx,vy))
		AG_PUT_PIXEL2(agView->v, vx,vy, color);
}

/* Write a single pixel at widget-relative coordinates (RGB components). */
static __inline__ void
AG_WidgetPutPixelRGB(void *p, int wx, int wy, Uint8 r, Uint8 g, Uint8 b)
{
	AG_Widget *wid = AGWIDGET(p);
	int vx = wid->rView.x1 + wx;
	int vy = wid->rView.y1 + wy;
	
#ifdef HAVE_OPENGL
	if (agView->opengl) {
		AG_WidgetPutPixelRGB_GL(p, vx,vy, r,g,b);
	} else
#endif
	if (!AG_CLIPPED_PIXEL(agView->v, vx, vy))
		AG_PUT_PIXEL2(agView->v, vx,vy, AG_MapRGB(agVideoFmt,r,g,b));
}

/* Blend a single pixel at widget-relative coordinates (32-bit agVideoFmt). */
static __inline__ void
AG_WidgetBlendPixel32(void *p, int wx, int wy, Uint32 pixel, AG_BlendFn fn)
{
	Uint8 c[4];

	AG_GetRGBA(pixel, agSurfaceFmt, &c[0],&c[1],&c[2],&c[3]);
	AG_WidgetBlendPixelRGBA(p, wx,wy, c, fn);
}

/* Expand widget to fill available space in parent container. */
static __inline__ void
AG_Expand(void *wid)
{
	AG_ObjectLock(wid);
	AGWIDGET(wid)->flags |= AG_WIDGET_EXPAND;
	AG_ObjectUnlock(wid);
}
static __inline__ void
AG_ExpandHoriz(void *wid)
{
	AG_ObjectLock(wid);
	AGWIDGET(wid)->flags |= AG_WIDGET_HFILL;
	AG_ObjectUnlock(wid);
}
static __inline__ void
AG_ExpandVert(void *wid)
{
	AG_ObjectLock(wid);
	AGWIDGET(wid)->flags |= AG_WIDGET_VFILL;
	AG_ObjectUnlock(wid);
}

/* Toggle widget visibility */
static __inline__ void
AG_WidgetHide(void *wid)
{
	AG_ObjectLock(wid);
	AGWIDGET(wid)->flags |= AG_WIDGET_HIDE;
	AG_ObjectUnlock(wid);
}
static __inline__ void
AG_WidgetShow(void *wid)
{
	AG_ObjectLock(wid);
	AGWIDGET(wid)->flags &= ~(AG_WIDGET_HIDE);
	AG_ObjectUnlock(wid);
}

/* Signal a change in a widget surface. */
#ifdef HAVE_OPENGL
# define AG_WidgetUpdateSurface(wid,name) do { \
	 AGWIDGET(wid)->surfaceFlags[(name)] |= AG_WIDGET_SURFACE_REGEN; \
} while (0)
#else
# define AG_WidgetUpdateSurface(wid,name)
#endif

/*
 * Request that all computed widget coordinates and geometries in the widget's
 * current window be updated as soon as possible. The widget may or may not
 * be currently attached to a window at the time the call is made.
 */
static __inline__ void
AG_WidgetUpdate(void *obj)
{
	AG_Widget *wid = (AG_Widget *)obj;

	AG_ObjectLock(wid);
	wid->flags |= AG_WIDGET_UPDATE_WINDOW;
	AG_ObjectUnlock(wid);
}
__END_DECLS

#ifdef AG_LEGACY
# include <agar/gui/widget_legacy.h>
#endif /* AG_LEGACY */

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_H_ */