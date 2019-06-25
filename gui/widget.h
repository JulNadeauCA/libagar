/*	Public domain	*/

#ifndef _AGAR_GUI_WIDGET_H_
#define _AGAR_GUI_WIDGET_H_

#include <agar/config/have_opengl.h>

#include <agar/gui/gui.h>
#include <agar/gui/geometry.h>
#include <agar/gui/colors.h>
#include <agar/gui/surface.h>
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
	struct ag_object_class _inherit;     /* [AG_Object] -> [AG_Widget] */

	void (*_Nullable draw)(void *_Nonnull);
	void (*_Nullable size_request)(void *_Nonnull, AG_SizeReq *_Nonnull);
	int  (*_Nullable size_allocate)(void *_Nonnull,
	                                const AG_SizeAlloc *_Nonnull);
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
	const char *_Nonnull descr;	/* Description (UTF-8) */
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
	struct ag_widget *_Nonnull widget;	/* Back pointer to widget */
	AG_Event *_Nullable fn;			/* Callback function */
	void *_Nullable p;			/* Target (for SET_*) */
	int val;				/* Value (for SET_*) */
	Uint bitmask;				/* Bitmask (for SET_FLAG) */
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
	struct ag_cursor *_Nullable c;			/* Associated cursor */
	struct ag_widget *_Nonnull wid;			/* Associated widget */
	int stock;					/* Stock cursor? */
	AG_TAILQ_ENTRY(ag_cursor_area) cursorAreas;
} AG_CursorArea;

/*
 * Widget's color palette. Generated from CSS style attributes.
 * SYNC: agWidgetStateNames[], agWidgetColorNames[], agDefaultPalette[].
 */
#define AG_WIDGET_NSTATES 5
#define AG_WIDGET_NCOLORS 5
enum ag_widget_state {
	AG_DEFAULT_STATE,		/* Unfocused state */
	AG_DISABLED_STATE,		/* Inactive state (#disabled) */
	AG_FOCUSED_STATE,		/* Active / focused state (#focused) */
	AG_HOVER_STATE,			/* "Mouse over" state (#hover) */
	AG_SELECTED_STATE		/* "Selected" state (#selected) */
};
enum ag_widget_color {
	AG_BG_COLOR = 0,		/* Background ("color") */
	AG_TEXT_COLOR,			/* Rendered text ("text-color") */
	AG_LINE_COLOR,			/* Line drawing ("line-color") */
	AG_SHAPE_COLOR,			/* Filled shapes ("shape-color") */
	AG_BORDER_COLOR			/* Decorative borders ("border-color") */
};
typedef struct {
	AG_Color c[AG_WIDGET_NSTATES]
	          [AG_WIDGET_NCOLORS];
} AG_WidgetPalette;

#define AG_WCOLOR(wid,which)	 AGWIDGET(wid)->pal.c[AGWIDGET(wid)->state][which]
#define AG_WCOLOR_DEF(wid,which) AGWIDGET(wid)->pal.c[AG_DEFAULT_STATE][which]
#define AG_WCOLOR_DIS(wid,which) AGWIDGET(wid)->pal.c[AG_DISABLED_STATE][which]
#define AG_WCOLOR_HOV(wid,which) AGWIDGET(wid)->pal.c[AG_HOVER_STATE][which]
#define AG_WCOLOR_SEL(wid,which) AGWIDGET(wid)->pal.c[AG_SELECTED_STATE][which]

#ifdef HAVE_OPENGL
/* Saved OpenGL context (for WIDGET_USE_OPENGL) */
typedef struct ag_widget_gl {
	float mProjection[16];				/* Projection matrix */
	float mModelview[16];				/* Modelview matrix */
} AG_WidgetGL;
#endif

typedef struct ag_widget_pvt {
	AG_Tbl actions;				 	/* Registered actions */
	AG_TAILQ_HEAD_(ag_action_tie) mouseActions;	/* Mouse action ties */
	AG_TAILQ_HEAD_(ag_action_tie) keyActions;	/* Kbd action ties */
#ifdef AG_TIMERS
	AG_TAILQ_HEAD_(ag_redraw_tie) redrawTies;	/* For AG_RedrawOn*() */
#endif
	AG_TAILQ_HEAD_(ag_cursor_area) cursorAreas;	/* Cursor-change areas */
} AG_WidgetPvt;

/* Agar widget instance */
typedef struct ag_widget {
	struct ag_object obj;			/* AG_Object -> AG_Widget */

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
#define AG_WIDGET_CATCH_TAB		0x001000 /* Receive focus-cycling key events */
#define AG_WIDGET_GL_RESHAPE		0x002000 /* Pending GL view reshape */
#define AG_WIDGET_UNDERSIZE		0x004000 /* Size alloc failed (computed) */
#define AG_WIDGET_NOSPACING		0x008000 /* No box model (container-specific) */
#define AG_WIDGET_UNFOCUSED_KEYDOWN	0x010000 /* Receive keydowns w/o focus */
#define AG_WIDGET_UNFOCUSED_KEYUP	0x020000 /* Receive keyups w/o focus */
#define AG_WIDGET_TABLE_EMBEDDABLE	0x080000 /* Can be used in a polled AG_Table(3) */
#define AG_WIDGET_UPDATE_WINDOW		0x100000 /* Request AG_WindowUpdate() ASAP */
#define AG_WIDGET_QUEUE_SURFACE_BACKUP	0x200000 /* Must backup surfaces ASAP */
#define AG_WIDGET_USE_TEXT		0x400000 /* Use Agar's font engine */
#define AG_WIDGET_USE_MOUSEOVER		0x800000 /* Generate mouseover events (and maintain MOUSEOVER flag) */
#define AG_WIDGET_EXPAND		(AG_WIDGET_HFILL|AG_WIDGET_VFILL)

	int x, y;			/* Coordinates in container */
	int w, h;			/* Allocated geometry */
	AG_Rect2 rView;			/* Effective view coordinates */
	AG_Rect2 rSens;			/* Effective sensitivity rectangle */

	AG_Surface *_Nullable *_Nullable surfaces;     /* Mapped surfaces */
	Uint8 *_Nullable                 surfaceFlags; /* Mapped surface flags: */
#define AG_WIDGET_SURFACE_NODUP	0x01                   /* Don't auto-free */
#define AG_WIDGET_SURFACE_REGEN	0x02                   /* Regenerate textures */
	Uint nSurfaces;

	Uint        *_Nullable textures;       /* Cached hardware textures */
	AG_TexCoord *_Nullable texcoords;      /* Cached texture coordinates */

	struct ag_widget *_Nullable focusFwd;     /* For ForwardFocus() */
	struct ag_window *_Nullable window;       /* Parent window (or null) */
	struct ag_driver *_Nullable drv;          /* Parent driver instance (null = no parent window) */
	struct ag_driver_class *_Nullable drvOps; /* Parent driver class (null = no parent window) */

	AG_StyleSheet *_Nullable css;      /* Style sheet (null = use default) */
	enum ag_widget_state state;       /* Current state for styling */
	struct ag_font *_Nullable font;    /* Computed font reference */
	AG_WidgetPalette pal;              /* Computed color palette */
#ifdef HAVE_OPENGL
	AG_WidgetGL *_Nullable gl;      /* Saved GL context (for USE_OPENGL) */
#endif
	AG_WidgetPvt pvt;               /* Private data */
} AG_Widget;

#define AGWIDGET(p)            ((AG_Widget *)(p))
#define AG_WIDGET_SELF()       AG_OBJECT(0,"AG_Widget:*")
#define AG_WIDGET_PTR(n)       AG_OBJECT((n),"AG_Widget:*")
#define AG_WIDGET_NAMED(n)     AG_OBJECT_NAMED((n),"AG_Widget:*")
#define AG_CONST_WIDGET_SELF() AG_CONST_OBJECT(0,"AG_Widget:*")
#define AG_CONST_WIDGET_PTR(n) AG_CONST_OBJECT((n),"AG_Widget:*")

#define AGWIDGET_OPS(wi)	((AG_WidgetClass *)AGOBJECT(wi)->cls)
#define AGWIDGET_SUPER_OPS(wi)	((AG_WidgetClass *)AGOBJECT(wi)->cls->super)

#define AGWIDGET_SURFACE(wi, ind)	AGWIDGET(wi)->surfaces[ind]
#define AGWIDGET_TEXTURE(wi, ind)	AGWIDGET(wi)->textures[ind]
#define AGWIDGET_SURFACE_NODUP(wi, ind)	(AGWIDGET(wi)->surfaceFlags[ind] & \
					 AG_WIDGET_SURFACE_NODUP)

#define AGWIDGET_KEYBOARD(obj) \
    (((obj) != NULL) ? AGWIDGET(obj)->drv->kbd : \
     (agDriverSw != NULL) ? AGDRIVER(agDriverSw)->kbd: NULL)

#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_GUI)
# define WIDGET(wi)			AGWIDGET(wi)
# define WIDGET_OPS(wi)			AGWIDGET_OPS(wi)
# define WIDGET_SUPER_OPS(wi)		AGWIDGET_SUPER_OPS(wi)
# define WSURFACE(wi,ind)		AGWIDGET_SURFACE((wi),(ind))
# define WTEXTURE(wi,ind)		AGWIDGET_TEXTURE((wi),(ind))
# define WSURFACE_NODUP(wi,ind)		AGWIDGET_SURFACE_NODUP((wi),(ind))
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
extern const char *_Nullable agWidgetStyleNames[];
extern const char *_Nullable agWidgetStateNames[];
extern const char *_Nullable agWidgetColorNames[];
extern AG_WidgetPalette agDefaultPalette;

void AG_WidgetDraw(void *_Nonnull);
void AG_WidgetSizeReq(void *_Nonnull, AG_SizeReq *_Nonnull);
void AG_WidgetSizeAlloc(void *_Nonnull, AG_SizeAlloc *_Nonnull);
int  AG_WidgetSetFocusable(void *_Nonnull, int);
void AG_WidgetForwardFocus(void *_Nonnull, void *_Nonnull);
int  AG_WidgetFocus(void *_Nonnull);
void AG_WidgetUnfocus(void *_Nonnull);

AG_Widget *_Nullable AG_WidgetFindFocused(void *_Nonnull);
void      *_Nullable AG_WidgetFindPoint(const char *_Nonnull, int,int);
void      *_Nullable AG_WidgetFindRect(const char *_Nonnull, int,int, int,int);

void AG_WidgetUpdateCoords(void *_Nonnull, int,int);
int  AG_WidgetMapSurface(void *_Nonnull, AG_Surface *_Nullable);
void AG_WidgetReplaceSurface(void *_Nonnull, int, AG_Surface *_Nullable);
void AG_WidgetUpdateSurface(void *_Nonnull, int);
void AG_WidgetUnmapSurface(void *_Nonnull, int);
void AG_WidgetBlitSurface(void *_Nonnull, int, int,int);

#ifdef HAVE_OPENGL
void AG_WidgetBlitGL(void *_Nonnull, AG_Surface *_Nonnull, float,float);
void AG_WidgetBlitSurfaceGL(void *_Nonnull, int, float,float);
void AG_WidgetBlitSurfaceFlippedGL(void *_Nonnull, int, float,float);
void AG_WidgetFreeResourcesGL(void *_Nonnull);
void AG_WidgetRegenResourcesGL(void *_Nonnull);
#endif

int         AG_WidgetSensitive(void *_Nonnull, int,int);
AG_SizeSpec AG_WidgetParseSizeSpec(const char *_Nonnull, int *_Nonnull);

void *_Nullable AG_WidgetFind(void *_Nonnull, const char *_Nonnull);

void AG_WidgetShow(void *_Nonnull);
void AG_WidgetHide(void *_Nonnull);
void AG_WidgetShowAll(void *_Nonnull);
void AG_WidgetHideAll(void *_Nonnull);

void AG_WidgetInheritDraw(void *_Nonnull);
void AG_WidgetInheritSizeRequest(void *_Nonnull, AG_SizeReq *_Nonnull);
int  AG_WidgetInheritSizeAllocate(void *_Nonnull, const AG_SizeAlloc *_Nonnull);

AG_Surface *_Nullable AG_WidgetSurface(void *_Nonnull);

void AG_RedrawOnChange(void *_Nonnull, int, const char *_Nonnull);
void AG_RedrawOnTick(void *_Nonnull, int);

void AG_WidgetStdKeyDown(AG_Event *_Nonnull);
void AG_WidgetStdKeyUp(AG_Event *_Nonnull);
void AG_WidgetStdMouseButtonDown(AG_Event *_Nonnull);
void AG_WidgetStdMouseButtonUp(AG_Event *_Nonnull);

AG_Action *_Nonnull AG_ActionFn(void *_Nonnull, const char *_Nonnull,
                                _Nonnull AG_EventFn,
				const char *_Nullable, ...);
AG_Action *_Nonnull AG_ActionSetInt(void *_Nonnull, const char *_Nonnull,
                                    int *_Nonnull, int);
AG_Action *_Nonnull AG_ActionToggleInt(void *_Nonnull, const char *_Nonnull,
                                       int *_Nonnull);
AG_Action *_Nonnull AG_ActionSetFlag(void *_Nonnull, const char *_Nonnull,
				     Uint *_Nonnull, Uint, int);
AG_Action *_Nonnull AG_ActionToggleFlag(void *_Nonnull, const char *_Nonnull,
					Uint *_Nonnull, Uint);

void AG_ActionOnButtonDown(void  *_Nonnull, int, const char *_Nonnull);
void AG_ActionOnButton(void *_Nonnull, int, const char *_Nonnull);
void AG_ActionOnButtonUp(void *_Nonnull, int, const char *_Nonnull);
void AG_ActionOnKey(void *_Nonnull, AG_KeySym, AG_KeyMod, const char *_Nonnull);
void AG_ActionOnKeyDown(void *_Nonnull, AG_KeySym, AG_KeyMod, const char *_Nonnull);
void AG_ActionOnKeyUp(void *_Nonnull, AG_KeySym, AG_KeyMod, const char *_Nonnull);

int AG_ExecMouseAction(void *_Nonnull, AG_ActionEventType, int, int, int);
int AG_ExecKeyAction(void *_Nonnull, AG_ActionEventType, AG_KeySym, AG_KeyMod);
int AG_ExecAction(void *_Nonnull, AG_Action *_Nonnull);

void AG_WidgetEnable(void *_Nonnull);
void AG_WidgetDisable(void *_Nonnull);

void AG_WidgetCompileStyle(void *_Nonnull);
void AG_WidgetCopyStyle(void *_Nonnull, void *_Nonnull);
void AG_WidgetFreeStyle(void *_Nonnull);

void AG_SetFont(void *_Nonnull, const struct ag_font *_Nonnull);
void AG_SetStyle(void *_Nonnull, const char *_Nonnull, const char *_Nullable);
void AG_SetStyleF(void *_Nonnull, const char *_Nonnull, const char *_Nullable, ...)
		 FORMAT_ATTRIBUTE(printf,3,4);

/*
 * Inlinables
 */
#ifdef AG_INLINE_WIDGET
# define AG_INLINE_HEADER
# include <agar/gui/inline_widget.h>
#else /* !AG_INLINE_WIDGET */

int  ag_widget_enabled(void *_Nonnull) _Pure_Attribute;
int  ag_widget_disabled(void *_Nonnull) _Pure_Attribute;
int  ag_widget_visible(void *_Nonnull) _Pure_Attribute;
int  ag_widget_is_focused_in_window(void *_Nonnull) _Pure_Attribute;
int  ag_widget_area(void *_Nonnull, int,int) _Pure_Attribute;
int  ag_widget_relative_area(void *_Nonnull, int,int) _Pure_Attribute;
void ag_expand(void *_Nonnull);
void ag_expand_horiz(void *_Nonnull);
void ag_expand_vert(void *_Nonnull);
void ag_widget_update(void *_Nonnull);
void ag_push_clip_rect(void *_Nonnull, const AG_Rect *_Nonnull);
void ag_pop_clip_rect(void *_Nonnull);
void ag_push_blending_mode(void *_Nonnull, AG_AlphaFn, AG_AlphaFn);
void ag_pop_blending_mode(void *_Nonnull);
int  ag_widget_map_surface_nodup(void *_Nonnull, AG_Surface *_Nonnull);
void ag_widget_replace_surface_nodup(void *_Nonnull, int, AG_Surface *_Nullable);
void ag_widget_blit(void *_Nonnull, AG_Surface *_Nonnull, int,int);
void ag_widget_blit_from(void *_Nonnull, int, AG_Rect *_Nullable, int,int);
void ag_set_key_state(void *_Nonnull, int *_Nonnull);
int *_Nonnull ag_get_key_state(void *_Nonnull) _Pure_Attribute;
int  ag_get_key_count(void *_Nonnull) _Pure_Attribute;
Uint ag_get_mod_state(void *_Nonnull) _Pure_Attribute;
void ag_set_mod_state(void *_Nonnull, Uint);

# define AG_WidgetEnabled(o)                 ag_widget_enabled(o)
# define AG_WidgetDisabled(o)                ag_widget_disabled(o)
# define AG_WidgetVisible(o)                 ag_widget_visible(o)
# define AG_WidgetIsFocusedInWindow(o)       ag_widget_is_focused_in_window(o)
# define AG_WidgetArea(o,x,y)                ag_widget_area((o),(x),(y))
# define AG_WidgetRelativeArea(o,x,y)        ag_widget_relative_area((o),(x),(y))
# define AG_Expand(o)                        ag_expand(o)
# define AG_ExpandHoriz(o)                   ag_expand_horiz(o)
# define AG_ExpandVert(o)                    ag_expand_vert(o)
# define AG_WidgetUpdate(o)                  ag_widget_update(o)
# define AG_PushClipRect(o,r)                ag_push_clip_rect((o),(r))
# define AG_PopClipRect(o)                   ag_pop_clip_rect(o)
# define AG_PushBlendingMode(o,fs,fd)        ag_push_blending_mode((o),(fs),(fd))
# define AG_PopBlendingMode(o)               ag_pop_blending_mode(o)
# define AG_WidgetMapSurfaceNODUP(o,S)       ag_widget_map_surface_nodup((o),(S))
# define AG_WidgetReplaceSurfaceNODUP(o,n,S) ag_widget_replace_surface_nodup((o),(n),(S))
# define AG_WidgetBlit(o,S,x,y)              ag_widget_blit((o),(S),(x),(y))
# define AG_WidgetBlitFrom(o,n,r,x,y)        ag_widget_blit_from((o),(n),(r),(x),(y))
# define AG_GetKeyState(o)                   ag_get_key_state(o)
# define AG_SetKeyState(o,ks)                ag_set_key_state((o),(ks))
# define AG_GetKeyCount(o)                   ag_get_key_count(o)
# define AG_GetModState(o)                   ag_get_mod_state(o)
# define AG_SetModState(o)                   ag_set_mod_state((o),(ms))
#endif /* AG_INLINE_WIDGET */
__END_DECLS

#ifdef AG_LEGACY
# include <agar/gui/widget_legacy.h>
#endif /* AG_LEGACY */

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_WIDGET_H_ */
