/*	Public domain	*/

#ifndef _AGAR_GUI_WIDGET_H_
#define _AGAR_GUI_WIDGET_H_

#include <agar/config/have_sdl.h>
#include <agar/config/have_opengl.h>

#include <agar/gui/gui.h>
#include <agar/gui/geometry.h>
#include <agar/gui/colors.h>
#include <agar/gui/surface.h>
#include <agar/gui/anim.h>
#include <agar/gui/stylesheet.h>
#include <agar/gui/mouse.h>
#include <agar/gui/keyboard.h>
#include <agar/gui/drv.h>

#include <agar/gui/begin.h>

struct ag_widget;
struct ag_cursor;
struct ag_font;

/* Widget size requisition and allocation. */
typedef struct ag_size_req {
	int w, h;			/* Requested geometry in pixels */
} AG_SizeReq;
typedef struct ag_size_alloc {
	int w, h;			/* Allocated geometry in pixels */
	int x, y;			/* Allocated position in pixels */
} AG_SizeAlloc;

/* Widget class description. */
typedef struct ag_widget_class {
	struct ag_object_class _inherit;
	void (*draw)(void *);
	void (*size_request)(void *, AG_SizeReq *);
	int  (*size_allocate)(void *, const AG_SizeAlloc *);
} AG_WidgetClass;

/* Relative size specification of visual element. */
typedef enum ag_widget_sizespec {
	AG_WIDGET_BAD_SPEC,		/* Parser error */
	AG_WIDGET_PIXELS,		/* Pixel count */
	AG_WIDGET_PERCENT,		/* % of available space */
	AG_WIDGET_STRINGLEN,		/* Width of given string */
	AG_WIDGET_FILL			/* Fill remaining space */
} AG_SizeSpec;

/* Packing mode (i.e., for container widgets). */
enum ag_widget_packing {
	AG_PACK_HORIZ,
	AG_PACK_VERT
};

/* Flag description (i.e., for AG_Checkbox(3)) */
typedef struct ag_flag_descr {
	Uint bitmask;			/* Bitmask */
	const char *descr;		/* Bit(s) description */
	int writeable;			/* User-editable */
} AG_FlagDescr;

/* 
 * Widget Actions
 */
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
	AG_ACTION_ON_KEYUP,		/* On key-up */
	AG_ACTION_ON_KEYREPEAT		/* On key-down, with key repeat */
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
			AG_Timer toRepeat;	/* Key repeat timer */
		} key;
	} data;
	char action[64];			/* Action name */
	AG_TAILQ_ENTRY(ag_action_tie) ties;
} AG_ActionTie;

/* Redraw tie (for AG_RedrawOn*() feature). */
enum ag_redraw_tie_type {
	AG_REDRAW_ON_CHANGE,
	AG_REDRAW_ON_TICK
};
typedef struct ag_redraw_tie {
	enum ag_redraw_tie_type type;
	char name[AG_VARIABLE_NAME_MAX];	/* Polled variable */
	AG_Variable Vlast;			/* Last accessed data */
	int         VlastInited;
	AG_Timer to;				/* Polling timer */
	Uint ival;				/* Polling interval */
	AG_TAILQ_ENTRY(ag_redraw_tie) redrawTies; /* In widget */
} AG_RedrawTie;

/* Cursor-change area */
typedef struct ag_cursor_area {
	AG_Rect r;					/* Area in window */
	struct ag_cursor *c;				/* Associated cursor */
	struct ag_widget *wid;				/* Associated widget */
	int stock;					/* Stock cursor? */
	AG_TAILQ_ENTRY(ag_cursor_area) cursorAreas;
} AG_CursorArea;

/*
 * Standard widget colors (CSS-defined, state-dependent, globally-inheritable).
 *
 * SYNC: agWidgetStateNames[], agWidgetColorNames[], agDefaultPalette[].
 */
#define AG_WIDGET_NSTATES 5
#define AG_WIDGET_NCOLORS 5
enum ag_widget_color_state {
	AG_DEFAULT_STATE,		/* Unfocused state */
	AG_DISABLED_STATE,		/* Inactive state (#disabled) */
	AG_FOCUSED_STATE,		/* Active / focused state (#focused) */
	AG_HOVER_STATE,			/* "Mouse over" state (#hover) */
	AG_SELECTED_STATE		/* "Selected" state (#selected) */
};
enum ag_widget_color {
	AG_COLOR = 0,			/* Background ("color") */
	AG_TEXT_COLOR,			/* Rendered text ("text-color") */
	AG_LINE_COLOR,			/* Line drawing ("line-color") */
	AG_SHAPE_COLOR,			/* Filled shapes ("shape-color") */
	AG_BORDER_COLOR			/* Decorative borders ("border-color") */
};
typedef struct {
	AG_Color c[AG_WIDGET_NSTATES][AG_WIDGET_NCOLORS];
} AG_WidgetPalette;
#define AG_WCOLOR(wid,which)	 AGWIDGET(wid)->pal.c[AGWIDGET(wid)->cState][which]
#define AG_WCOLOR_DEF(wid,which) AGWIDGET(wid)->pal.c[AG_DEFAULT_STATE][which]
#define AG_WCOLOR_DIS(wid,which) AGWIDGET(wid)->pal.c[AG_DISABLED_STATE][which]
#define AG_WCOLOR_HOV(wid,which) AGWIDGET(wid)->pal.c[AG_HOVER_STATE][which]
#define AG_WCOLOR_SEL(wid,which) AGWIDGET(wid)->pal.c[AG_SELECTED_STATE][which]

/* Base Agar widget */
typedef struct ag_widget {
	struct ag_object obj;

	Uint flags;
#define AG_WIDGET_FOCUSABLE		0x000001 /* Can grab focus */
#define AG_WIDGET_FOCUSED		0x000002 /* Holds focus (computed) */
#define AG_WIDGET_UNFOCUSED_MOTION	0x000004 /* All mousemotion events */
#define AG_WIDGET_UNFOCUSED_BUTTONUP	0x000008 /* All mousebuttonup events */
#define AG_WIDGET_UNFOCUSED_BUTTONDOWN	0x000010 /* All mousebuttondown events */
#define AG_WIDGET_VISIBLE		0x000020 /* Widget is visible (computed) */
#define AG_WIDGET_HFILL			0x000040 /* Expand to fill width */
#define AG_WIDGET_VFILL			0x000080 /* Expand to fill height */
#define AG_WIDGET_USE_OPENGL		0x000100 /* Set up separate GL context */
#define AG_WIDGET_HIDE			0x000200 /* Don't draw this widget */
#define AG_WIDGET_DISABLED		0x000400 /* Don't respond to input */
#define AG_WIDGET_MOUSEOVER		0x000800 /* Mouseover state (computed) */
#define AG_WIDGET_CATCH_TAB		0x001000 /* Catch tab key events */
#define AG_WIDGET_GL_RESHAPE		0x002000 /* Pending GL view reshape */
#define AG_WIDGET_UNDERSIZE		0x004000 /* Size alloc failed (computed) */
#define AG_WIDGET_NOSPACING		0x008000 /* No box model (container-specific) */
#define AG_WIDGET_UNFOCUSED_KEYDOWN	0x010000 /* All mousebuttondown events */
#define AG_WIDGET_UNFOCUSED_KEYUP	0x020000 /* All mousebuttondown events */
#define AG_WIDGET_TABLE_EMBEDDABLE	0x080000 /* Usable in polled tables */
#define AG_WIDGET_UPDATE_WINDOW		0x100000 /* Request an AG_WindowUpdate() as soon as possible */
#define AG_WIDGET_QUEUE_SURFACE_BACKUP	0x200000 /* Must backup surfaces ASAP */
#define AG_WIDGET_USE_TEXT		0x400000 /* Use Agar's font engine */
#define AG_WIDGET_USE_MOUSEOVER		0x800000 /* Update MOUSEOVER flag and generate mouseover events */
#define AG_WIDGET_EXPAND		(AG_WIDGET_HFILL|AG_WIDGET_VFILL)

	int x, y;			/* Coordinates in container */
	int w, h;			/* Allocated geometry */
	AG_Rect2 rView;			/* Computed view coordinates */
	AG_Rect2 rSens;			/* Cursor notification area */
	AG_Surface **surfaces;		/* Registered surfaces */
	Uint        *surfaceFlags;	/* Surface flags */
#define AG_WIDGET_SURFACE_NODUP	0x01	/* Don't free on destroy */
#define AG_WIDGET_SURFACE_REGEN	0x02	/* Texture needs to be regenerated */
	Uint        nsurfaces;
	Uint        *textures;		/* Cached textures (driver-specific) */
	AG_TexCoord *texcoords;		/* Cached texture coordinates */

	struct ag_widget *focusFwd;		/* For ForwardFocus() */
	struct ag_window *window;		/* Back ptr to parent window */
	struct ag_driver *drv;			/* Back ptr to driver */
	struct ag_driver_class *drvOps;		/* Back ptr to driver class */

	AG_Tbl actions;				 	/* Registered actions */
	AG_TAILQ_HEAD_(ag_action_tie) mouseActions;	/* Mouse action ties */
	AG_TAILQ_HEAD_(ag_action_tie) keyActions;	/* Kbd action ties */
	AG_TAILQ_HEAD_(ag_redraw_tie) redrawTies;	/* For AG_RedrawOn*() */
	AG_TAILQ_HEAD_(ag_cursor_area) cursorAreas;	/* Cursor-change areas
							   (not yet attached) */

	AG_StyleSheet *css;			/* Alternate style sheet */
	enum ag_widget_color_state cState;	/* Current CSS color state */
	struct ag_font *font;			/* Computed font reference */
	AG_WidgetPalette pal;			/* Computed color palette */

	struct {
		float mProjection[16];		/* Projection matrix */
		float mModelview[16];		/* Modelview matrix */
	} gl;
} AG_Widget;

#define AGWIDGET(wi)		((AG_Widget *)(wi))
#define AGWIDGET_OPS(wi)	((AG_WidgetClass *)AGOBJECT(wi)->cls)
#define AGWIDGET_SUPER_OPS(wi)	((AG_WidgetClass *)AGOBJECT(wi)->cls->super)

#define AGWIDGET_SURFACE(wi, ind)	AGWIDGET(wi)->surfaces[ind]
#define AGWIDGET_TEXTURE(wi, ind)	AGWIDGET(wi)->textures[ind]
#define AGWIDGET_SURFACE_NODUP(wi, ind)	(AGWIDGET(wi)->surfaceFlags[ind] & \
					 AG_WIDGET_SURFACE_NODUP)
#define AGSTYLE(p)			 AGWIDGET(p)->theme

#define AGWIDGET_KEYBOARD(obj)				\
    (((obj) != NULL) ? AGWIDGET(obj)->drv->kbd :	\
     (agDriverSw != NULL) ? AGDRIVER(agDriverSw)->kbd:	\
     NULL)

#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_GUI)
# define WIDGET(wi)			AGWIDGET(wi)
# define WIDGET_OPS(wi)			AGWIDGET_OPS(wi)
# define WIDGET_SUPER_OPS(wi)		AGWIDGET_SUPER_OPS(wi)
# define WSURFACE(wi,ind)		AGWIDGET_SURFACE((wi),(ind))
# define WTEXTURE(wi,ind)		AGWIDGET_TEXTURE((wi),(ind))
# define WSURFACE_NODUP(wi,ind)		AGWIDGET_SURFACE_NODUP((wi),(ind))
# define STYLE(p)			AGSTYLE(p)
# define WIDTH(p)			AGWIDGET(p)->w
# define HEIGHT(p)			AGWIDGET(p)->h
# define WCOLOR(wid,which)		AG_WCOLOR((wid),(which))
# define WCOLOR_DEF(wid,which)		AG_WCOLOR_DEF((wid),(which))
# define WCOLOR_DIS(wid,which)		AG_WCOLOR_DIS((wid),(which))
# define WCOLOR_HOV(wid,which)		AG_WCOLOR_HOV((wid),(which))
# define WCOLOR_SEL(wid,which)		AG_WCOLOR_SEL((wid),(which))
# define TEXT_COLOR			AG_TEXT_COLOR
# define LINE_COLOR			AG_LINE_COLOR
# define SHAPE_COLOR			AG_SHAPE_COLOR
# define BORDER_COLOR			AG_BORDER_COLOR
#endif /* _AGAR_INTERNAL or _USE_AGAR_GUI */

struct ag_window;
struct ag_font;
AG_TAILQ_HEAD(ag_widgetq, ag_widget);

__BEGIN_DECLS
extern AG_WidgetClass agWidgetClass;
extern const char *agWidgetPropNames[];
extern const char *agWidgetStateNames[];
extern const char *agWidgetColorNames[];
extern AG_WidgetPalette agDefaultPalette;

void       AG_WidgetDraw(void *);
void       AG_WidgetSizeReq(void *, AG_SizeReq *);
void       AG_WidgetSizeAlloc(void *, AG_SizeAlloc *);
void       AG_WidgetSetFocusable(void *, int);
void       AG_WidgetForwardFocus(void *, void *);

int        AG_WidgetFocus(void *);
void       AG_WidgetUnfocus(void *);
AG_Widget *AG_WidgetFindFocused(void *);
void      *AG_WidgetFindPoint(const char *, int, int);
void      *AG_WidgetFindRect(const char *, int, int, int, int);
void       AG_WidgetUpdateCoords(void *, int, int);

int	 AG_WidgetMapSurface(void *, AG_Surface *);
void	 AG_WidgetReplaceSurface(void *, int, AG_Surface *);
#define	 AG_WidgetUnmapSurface(w, n) \
	 AG_WidgetReplaceSurface((w),(n),NULL)
#define  AG_WidgetBlitSurface(p,n,x,y) \
	 AG_WidgetBlitFrom((p),(p),(n),NULL,(x),(y))
#ifdef HAVE_OPENGL
void	 AG_WidgetBlitGL(void *, AG_Surface *, float, float);
void	 AG_WidgetBlitSurfaceGL(void *, int, float, float);
void	 AG_WidgetBlitSurfaceFlippedGL(void *, int, float, float);
void	 AG_WidgetFreeResourcesGL(void *);
void	 AG_WidgetRegenResourcesGL(void *);
#endif

int         AG_WidgetSensitive(void *, int, int);
AG_SizeSpec AG_WidgetParseSizeSpec(const char *, int *);
int         AG_WidgetScrollDelta(Uint32 *);
void       *AG_WidgetFind(void *, const char *);

void        AG_WidgetShow(void *);
void        AG_WidgetHide(void *);
void        AG_WidgetShowAll(void *);
void        AG_WidgetHideAll(void *);
void        AG_WidgetEnableAll(void *);
void        AG_WidgetDisabledAll(void *);

void        AG_WidgetInheritDraw(void *);
void        AG_WidgetInheritSizeRequest(void *, AG_SizeReq *);
int         AG_WidgetInheritSizeAllocate(void *, const AG_SizeAlloc *);
AG_Surface *AG_WidgetSurface(void *);

void        AG_RedrawOnChange(void *, int, const char *);
void        AG_RedrawOnTick(void *, int);

void        AG_WidgetStdKeyDown(AG_Event *);
void        AG_WidgetStdKeyUp(AG_Event *);
void        AG_WidgetStdMouseButtonDown(AG_Event *);
void        AG_WidgetStdMouseButtonUp(AG_Event *);

AG_Action  *AG_ActionFn(void *, const char *, AG_EventFn, const char *, ...);
AG_Action  *AG_ActionSetInt(void *, const char *, int *, int);
AG_Action  *AG_ActionToggleInt(void *, const char *, int *);
AG_Action  *AG_ActionSetFlag(void *, const char *, Uint *, Uint, int);
AG_Action  *AG_ActionToggleFlag(void *, const char *, Uint *, Uint);

void        AG_ActionOnButtonDown(void *, int, const char *);
void        AG_ActionOnButtonUp(void *, int, const char *);
#define     AG_ActionOnButton(w,b,a) \
            AG_ActionOnButtonDown((w),(b),(a))

void        AG_ActionOnKey(void *, AG_KeySym, AG_KeyMod, const char *);
void        AG_ActionOnKeyDown(void *, AG_KeySym, AG_KeyMod, const char *);
void        AG_ActionOnKeyUp(void *, AG_KeySym, AG_KeyMod, const char *);

int         AG_ExecMouseAction(void *, AG_ActionEventType, int, int, int);
int         AG_ExecKeyAction(void *, AG_ActionEventType, AG_KeySym, AG_KeyMod);
int         AG_ExecAction(void *, AG_Action *);

void        AG_WidgetEnable(void *);
void        AG_WidgetDisable(void *);

void        AG_WidgetCompileStyle(void *);
void        AG_WidgetCopyStyle(void *, void *);
void        AG_WidgetFreeStyle(void *);

void        AG_SetFont(void *, const struct ag_font *);
void        AG_SetStyle(void *, const char *, const char *);

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

/* Return the widget's visibility state. The Widget object must be locked. */
static __inline__ int
AG_WidgetVisible(void *p)
{
	return (AGWIDGET(p)->flags & AG_WIDGET_VISIBLE);
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

/*
 * Push a clipping rectangle onto the stack, by widget-relative coordinates.
 * Must be invoked from GUI rendering context.
 */
static __inline__ void
AG_PushClipRect(void *obj, AG_Rect pr)
{
	AG_Widget *wid = (AG_Widget *)obj;
	AG_Rect r;

	r.x = wid->rView.x1 + pr.x;
	r.y = wid->rView.y1 + pr.y;
	r.w = pr.w;
	r.h = pr.h;
	wid->drvOps->pushClipRect(wid->drv, r);
}

/*
 * Pop a clipping rectangle off the clipping rectangle stack.
 * Must be invoked from GUI rendering context.
 */
static __inline__ void
AG_PopClipRect(void *obj)
{
	AG_Widget *wid = (AG_Widget *)obj;

	wid->drvOps->popClipRect(wid->drv);
}

/* Set the blending mode, pushing the current mode on a stack. */
static __inline__ void
AG_PushBlendingMode(void *obj, AG_BlendFn fnSrc, AG_BlendFn fnDst)
{
	AG_Widget *wid = (AG_Widget *)obj;

	wid->drvOps->pushBlendingMode(wid->drv, fnSrc, fnDst);
}

/* Restore the last blending mode. */
static __inline__ void
AG_PopBlendingMode(void *obj)
{
	AG_Widget *wid = (AG_Widget *)obj;
	
	wid->drvOps->popBlendingMode(wid->drv);
}

/* Offset the coordinates of an AG_Rect per widget coordinates. */
static __inline__ void
AG_WidgetOffsetRect(void *obj, AG_Rect *r)
{
	AG_Widget *wid = (AG_Widget *)obj;

	r->x += wid->rView.x1;
	r->y += wid->rView.y1;
}

/*
 * Variant of AG_WidgetMapSurface() that sets the NODUP flag such that
 * the surface is not freed by the widget.
 */
static __inline__ int
AG_WidgetMapSurfaceNODUP(void *obj, AG_Surface *su)
{
	AG_Widget *wid = (AG_Widget *)obj;
	int name;

	AG_ObjectLock(wid);
	if ((name = AG_WidgetMapSurface(wid, su)) != -1) {
		wid->surfaceFlags[name] |= AG_WIDGET_SURFACE_NODUP;
	}
	AG_ObjectUnlock(wid);
	return (name);
}

/* Variant of WidgetReplaceSurface() that sets the NODUP flag. */
static __inline__ void
AG_WidgetReplaceSurfaceNODUP(void *obj, int name, AG_Surface *su)
{
	AG_Widget *wid = (AG_Widget *)obj;

	AG_ObjectLock(wid);
#ifdef AG_DEBUG
	if (name < 0 || name >= (int)wid->nsurfaces)
		AG_FatalError("Bad surface handle");
#endif
	AG_WidgetReplaceSurface(wid, name, su);
	wid->surfaceFlags[name] |= AG_WIDGET_SURFACE_NODUP;
	AG_ObjectUnlock(wid);
}

/*
 * Draw an unmapped surface at given coordinates in the widget's coordinate
 * system. With hardware-accelerated drivers, this operation is slow compared
 * to drawing of mapped surfaces, since a software->hardware copy is done.
 */
static __inline__ void
AG_WidgetBlit(void *obj, AG_Surface *s, int x, int y)
{
	AG_Widget *wid = (AG_Widget *)obj;

	wid->drvOps->blitSurface(wid->drv, wid, s,
	    wid->rView.x1 + x,
	    wid->rView.y1 + y);
}

/*
 * Perform a hardware or software blit from a mapped surface to the display
 * at coordinates relative to the widget, using clipping.
 */
static __inline__ void
AG_WidgetBlitFrom(void *obj, void *objSrc, int s, AG_Rect *r, int x, int y)
{
	AG_Widget *wid = (AG_Widget *)obj;
	AG_Widget *widSrc = (AG_Widget *)objSrc;
	
	if (s == -1 || widSrc->surfaces[s] == NULL)
		return;

	wid->drvOps->blitSurfaceFrom(wid->drv, wid, widSrc, s, r,
	    wid->rView.x1 + x,
	    wid->rView.y1 + y);
}

/*
 * Routines for direct access to keyboard key and modifier state
 * from widget code.
 */
static __inline__ int *
AG_GetKeyState(void *obj)
{
	AG_Keyboard *kbd = AGWIDGET_KEYBOARD(obj);
	return (kbd->keyState);
}
static __inline__ void
AG_SetKeyState(void *obj, int *ks)
{
	AG_Keyboard *kbd = AGWIDGET_KEYBOARD(obj);
	memcpy(kbd->keyState, ks, kbd->keyCount*sizeof(int));
}
static __inline__ int
AG_GetKeyCount(void *obj)
{
	AG_Keyboard *kbd = AGWIDGET_KEYBOARD(obj);
	return (kbd->keyCount);
}
static __inline__ Uint
AG_GetModState(void *obj)
{
	AG_Keyboard *kbd = AGWIDGET_KEYBOARD(obj);
	return (kbd->modState);
}
static __inline__ void
AG_SetModState(void *obj, Uint ms)
{
	AG_Keyboard *kbd = AGWIDGET_KEYBOARD(obj);
	kbd->modState = ms;
}
__END_DECLS

#ifdef AG_LEGACY
# include <agar/gui/widget_legacy.h>
#endif /* AG_LEGACY */

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_WIDGET_H_ */
