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

#ifndef AG_ACTION_NAME_MAX
#define AG_ACTION_NAME_MAX 28
#endif

/*
 * Size Requisition. Child widgets use this structure to ask their parent
 * container widgets for a preferred initial geometry in pixels.
 */
typedef struct ag_size_req {
	int w, h;			/* Requested geometry in pixels */
} AG_SizeReq;

/*
 * Size Allocation. Parent container widgets use this structure to communicate
 * to their child widgets their final allocated position and size in pixels.
 */
typedef struct ag_size_alloc {
	int w, h;			/* Allocated geometry in pixels */
	int x, y;			/* Allocated position in pixels */
} AG_SizeAlloc;

/* Widget class description */
typedef struct ag_widget_class {
	struct ag_object_class _inherit;     /* [AG_Object] -> [AG_Widget] */

	/* Rendering routine (rendering context) */
	void (*_Nullable draw)(void *_Nonnull);
	
	/* Size requisition (indicate preferred initial size to parent) */
	void (*_Nullable size_request)(void *_Nonnull, AG_SizeReq *_Nonnull);

	/* Size allocation (handle final size returned by parent) */
	int  (*_Nullable size_allocate)(void *_Nonnull,
	                                const AG_SizeAlloc *_Nonnull);
} AG_WidgetClass;

/* A constant size for some visual element (possibly a relative to a parent) */
typedef enum ag_widget_sizespec {
	AG_WIDGET_BAD_SPEC,    /* Parser error */
	AG_WIDGET_PIXELS,      /* Pixel count ("123px", "1.5px") */
	AG_WIDGET_PERCENT,     /* Ratio to available space ("50%") */
	AG_WIDGET_STRINGLEN,   /* Width of rendered text ("<Hello...>") */
	AG_WIDGET_FILL         /* Expand to fill remaining space ("-") */
} AG_SizeSpec;

/* Bit flag description (used by AG_Checkbox(3) for example). */
typedef struct ag_flag_descr {
#ifdef AG_HAVE_64BIT
	Uint64 bitmask;			/* Bitmask */
#else
	Uint bitmask;			/* Bitmask */
#endif
	const char *_Nonnull descr;	/* Description (UTF-8) */
	int writeable;			/* User-editable */
	Uint32 _pad;
} AG_FlagDescr;

/* 
 * High-level widget action.
 */
typedef enum ag_action_type {
	AG_ACTION_FN,             /* Execute function */
	AG_ACTION_SET_INT,        /* Set an integer */
	AG_ACTION_TOGGLE_INT,     /* Toggle an integer */
	AG_ACTION_SET_FLAG,       /* Set specified bits in a word */
	AG_ACTION_TOGGLE_FLAG     /* Toggle specified bits in a word */
} AG_ActionType;

typedef struct ag_action {
	AG_ActionType type;              /* Type of action */
	char name[AG_ACTION_NAME_MAX];   /* Action name */
	AG_Event *_Nullable fn;          /* Callback function */
	void *_Nullable p;               /* Target (for SET_*) */
	int val;                         /* Value (for SET_*) */
	Uint bitmask;                    /* Bitmask (for SET_FLAG) */
} AG_Action;

typedef AG_VEC_HEAD(AG_Action *) AG_ActionVec;

/*
 * Connection between an Action and a low-level event.
 */
typedef enum ag_action_event_type {
	AG_ACTION_ON_BUTTONDOWN,         /* On mouse-button-down */
	AG_ACTION_ON_BUTTONUP,           /* On mouse-button-up */
	AG_ACTION_ON_KEYDOWN,            /* On key-down */
	AG_ACTION_ON_KEYUP,              /* On key-up */
	AG_ACTION_ON_KEYREPEAT           /* On key-down, with key repeat */
#define AG_ACTION_ON_BUTTON \
	AG_ACTION_ON_BUTTONDOWN          /* For mousewheel events */
} AG_ActionEventType;

typedef struct ag_action_tie {
	AG_ActionEventType type;               /* Trigger event type */
	char action[AG_ACTION_NAME_MAX];       /* Action name */
	union {
		AG_MouseButton button;         /* Button index */
		struct {
			AG_KeySym sym;         /* Matching symbol */
			AG_KeyMod mod;         /* Matching modifier */
			AG_Timer toRepeat;     /* Key repeat timer */
		} key;
	} data;
	AG_TAILQ_ENTRY(ag_action_tie) ties;
} AG_ActionTie;

/*
 * Instruction to redraw at specified interval in ticks, or whenever
 * the value of a given variable changes.
 */
enum ag_redraw_tie_type {
	AG_REDRAW_ON_CHANGE,
	AG_REDRAW_ON_TICK
};
typedef struct ag_redraw_tie {
	enum ag_redraw_tie_type type;
	char name[AG_VARIABLE_NAME_MAX];           /* Polled variable */
	AG_Variable Vlast;                         /* Last accessed data */
	int         VlastInited;
	Uint ival;                                 /* Polling interval */
	AG_Timer to;                               /* Polling timer */
	AG_TAILQ_ENTRY(ag_redraw_tie) redrawTies;  /* Entry in Widget */
} AG_RedrawTie;

/* Cursor-change area */
typedef struct ag_cursor_area {
	AG_Rect r;                           /* Area in window */
	struct ag_cursor *_Nullable c;       /* Associated cursor */
	struct ag_widget *_Nonnull wid;      /* Associated widget */
	int stock;                           /* Stock cursor? */
	Uint32 _pad;
	AG_TAILQ_ENTRY(ag_cursor_area) cursorAreas;
} AG_CursorArea;
 
/*
 * Widget states that effect styling.
 * SYNC: agWidgetStateNames[]
 */
enum ag_widget_state {
	AG_DEFAULT_STATE,    /* Not focused (default) */
	AG_DISABLED_STATE,   /* Inactive ("#disabled") */
	AG_FOCUSED_STATE,    /* Holds focus ("#focused")  */
	AG_HOVER_STATE       /* Cursor is over widget ("#hover") */
};
#define AG_WIDGET_NSTATES 4

/*
 * Per-widget 32-color palette (4 states x 8 colors).
 *
 * Initialized by the style engine. Read-only except in rendering context,
 * where container widgets are allowed to modify the palette entries of
 * child widgets.
 */
enum ag_widget_color {
	AG_FG_COLOR        = 0,  /*            "color" : Foreground primary */
	AG_BG_COLOR        = 1,  /* "background-color" : Background primary */
	AG_TEXT_COLOR      = 2,  /*       "text-color" : Text & vector icons */
	AG_LINE_COLOR      = 3,  /*       "line-color" : Lines & filled shapes */
	AG_HIGH_COLOR      = 4,  /*       "high-color" : Shading top & left */
	AG_LOW_COLOR       = 5,  /*        "low-color" : Shading bottom & right */
	AG_SELECTION_COLOR = 6,  /*  "selection-color" : Selection primary */
	AG_UNUSED_COLOR    = 7
};
#define AG_WIDGET_NCOLORS 8

typedef struct {
	AG_Color c[AG_WIDGET_NSTATES]
	          [AG_WIDGET_NCOLORS];
} AG_WidgetPalette;

/* Index a palette entry according to the current widget's state */
#define AG_WCOLOR(wid,id) AGWIDGET(wid)->pal.c[AGWIDGET(wid)->state][id]

/* Index a palette entry for a given widget state */
#define AG_WCOLOR_DEFAULT(wid,id)  AGWIDGET(wid)->pal.c[AG_DEFAULT_STATE][id]
#define AG_WCOLOR_DISABLED(wid,id) AGWIDGET(wid)->pal.c[AG_DISABLED_STATE][id]
#define AG_WCOLOR_FOCUSED(wid,id)  AGWIDGET(wid)->pal.c[AG_FOCUSED_STATE][id]
#define AG_WCOLOR_HOVER(wid,id)    AGWIDGET(wid)->pal.c[AG_HOVER_STATE][id]

#ifdef HAVE_OPENGL
/* Per-widget saved OpenGL context (for USE_OPENGL). */
typedef struct ag_widget_gl {
	float mProjection[16];                       /* Projection matrix */
	float mModelview[16];                        /* Modelview matrix */
} AG_WidgetGL;
#endif

/* Per-widget Private Data */
typedef struct ag_widget_pvt {
	AG_TAILQ_HEAD_(ag_action_tie) mouseActions;  /* Mouse action ties */
	AG_TAILQ_HEAD_(ag_action_tie) keyActions;    /* Kbd action ties */
	AG_TAILQ_HEAD_(ag_redraw_tie) redrawTies;    /* For AG_RedrawOn*() */
	AG_TAILQ_HEAD_(ag_cursor_area) cursorAreas;  /* Cursor-change areas */
} AG_WidgetPvt;

/*
 * Agar widget instance.
 */
typedef struct ag_widget {
	struct ag_object obj;                      /* AG_Object -> AG_Widget */

	Uint flags;
#define AG_WIDGET_FOCUSABLE             0x00000001 /* Can grab focus */
#define AG_WIDGET_FOCUSED               0x00000002 /* is currently focused (RO) */
#define AG_WIDGET_UNFOCUSED_MOTION      0x00000004 /* Receive all motion events */
#define AG_WIDGET_UNFOCUSED_BUTTONUP    0x00000008 /* Receive all buttonup events */
#define AG_WIDGET_UNFOCUSED_BUTTONDOWN  0x00000010 /* Receive all buttondown events */
#define AG_WIDGET_VISIBLE               0x00000020 /* is visible (RO) */
#define AG_WIDGET_HFILL                 0x00000040 /* Expand to fill width */
#define AG_WIDGET_VFILL                 0x00000080 /* Expand to fill height */
#define AG_WIDGET_USE_OPENGL            0x00000100 /* Create a private GL context */
#define AG_WIDGET_HIDE                  0x00000200 /* Don't display */
#define AG_WIDGET_DISABLED              0x00000400 /* Inactive state */
#define AG_WIDGET_MOUSEOVER             0x00000800 /* Cursor is hovering (RO) */
#define AG_WIDGET_CATCH_TAB             0x00001000 /* Receive focus-cycling key events */
#define AG_WIDGET_GL_RESHAPE            0x00002000 /* Pending GL view reshape */
#define AG_WIDGET_UNDERSIZE             0x00004000 /* Allocation could not be met (RO) */
                                     /* 0x00008000 */
#define AG_WIDGET_UNFOCUSED_KEYDOWN     0x00010000 /* Receive keydowns w/o focus */
#define AG_WIDGET_UNFOCUSED_KEYUP       0x00020000 /* Receive keyups w/o focus */
                                     /* 0x00040000 */
                                     /* 0x00080000 */
#define AG_WIDGET_UPDATE_WINDOW         0x00100000 /* Request WindowUpdate() ASAP */
#define AG_WIDGET_QUEUE_SURFACE_BACKUP  0x00200000 /* Software-backup surfaces now */
#define AG_WIDGET_USE_TEXT              0x00400000 /* Allow Text{Size,Render}() */
#define AG_WIDGET_USE_MOUSEOVER         0x00800000 /* Use MOUSEOVER flag & events */
#define AG_WIDGET_EXPAND               (AG_WIDGET_HFILL | AG_WIDGET_VFILL)

	int x, y;                          /* Coordinates in container */
	int w, h;                          /* Allocated geometry */

	AG_Rect  r;                        /* Rectangle at 0,0 (cached) */
	AG_Rect2 rView;                    /* Display coordinates (cached) */
	AG_Rect2 rSens;                    /* Cursor sensitivity rectangle */

	Uint                            nSurfaces; /* Mapped surface count */
	AG_Surface *_Nullable *_Nullable surfaces; /* Mapped surfaces */

	Uint *_Nullable surfaceFlags;      /* Per-surface flags: */
#define AG_WIDGET_SURFACE_NODUP	0x01       /* Do not SurfaceFree() on cleanup */
#define AG_WIDGET_SURFACE_REGEN	0x02       /* Request hardware texture update */

	Uint        *_Nullable textures;   /* Cached hardware textures */
	AG_TexCoord *_Nullable texcoords;  /* Cached texture coordinates */

	struct ag_widget *_Nullable focusFwd;     /* (TODO use a Variable) */ 
	struct ag_window *_Nullable window;       /* Parent window (NULL=none) */
	struct ag_driver *_Nullable drv;          /* Parent driver instance */
	struct ag_driver_class *_Nullable drvOps; /* Parent driver class */

	AG_StyleSheet *_Nullable css;       /* Style sheet (null = use default) */
	                                    /* (TODO use a Variable) */

	enum ag_widget_state state;         /* Style-effecting state */
	Uint8 marginTop, marginRight;       /* Margin (px outside of border) */
	Uint8 marginBottom, marginLeft;
	Uint paddingTop, paddingRight;      /* Padding (px around contents) */
	Uint paddingBottom, paddingLeft;
	Uint spacingHoriz, spacingVert;     /* Spacing (px between items) */

	struct ag_font *_Nullable font;    /* Active font (style-generated) */
	AG_WidgetPalette pal;              /* Color palette (style-generated) */
#ifdef HAVE_OPENGL
	AG_WidgetGL *_Nullable gl;      /* Saved GL context (for USE_OPENGL) */
#endif
	AG_ActionVec actions;           /* Registered Widget Actions */
	AG_WidgetPvt pvt;               /* Private data */
} AG_Widget;

typedef AG_VEC_HEAD(AG_Widget *) AG_WidgetVec;

#define AGWIDGET(p)             ((AG_Widget *)(p))
#define AGCWIDGET(p)            ((const AG_Widget *)(p))
#define AG_WIDGET_SELF()          AGWIDGET( AG_OBJECT(0,"AG_Widget:*") )
#define AG_WIDGET_PTR(n)          AGWIDGET( AG_OBJECT((n),"AG_Widget:*") )
#define AG_WIDGET_NAMED(n)        AGWIDGET( AG_OBJECT_NAMED((n),"AG_Widget:*") )
#define AG_CONST_WIDGET_SELF()   AGCWIDGET( AG_CONST_OBJECT(0,"AG_Widget:*") )
#define AG_CONST_WIDGET_PTR(n)   AGCWIDGET( AG_CONST_OBJECT((n),"AG_Widget:*") )
#define AG_CONST_WIDGET_NAMED(n) AGCWIDGET( AG_CONST_OBJECT((n),"AG_Widget:*") )

#define AGWIDGET_OPS(wi)          ((AG_WidgetClass *)AGOBJECT(wi)->cls)
#define AGWIDGET_SUPER_OPS(wi)    ((AG_WidgetClass *)AGOBJECT(wi)->cls->super)

#define AGWIDGET_SURFACE(wi, ind) AGWIDGET(wi)->surfaces[ind]
#define AGWIDGET_TEXTURE(wi, ind) AGWIDGET(wi)->textures[ind]
#ifdef AG_DEBUG
# define AGWIDGET_FONT(wi) ((AGWIDGET(wi)->flags & AG_WIDGET_USE_TEXT) ? \
                             AGWIDGET(wi)->font : (AG_Font *) \
                             AG_GenericMismatch("by AGWIDGET_FONT(). " \
                                                "Did you forget USE_TEXT?"))
#else
# define AGWIDGET_FONT(wi) AGWIDGET(wi)->font
#endif
#define AGWIDGET_KEYBOARD(obj) ((obj) ? AGWIDGET(obj)->drv->kbd : \
                                (agDriverSw) ? AGDRIVER(agDriverSw)->kbd : NULL)
#define AGWIDGET_MOUSE(obj) ((obj) ? AGWIDGET(obj)->drv->mouse : \
                             (agDriverSw) ? AGDRIVER(agDriverSw)->mouse : NULL)

#if defined(_AGAR_INTERNAL) || defined(_USE_AGAR_GUI)
# define WIDGET(wid)             AGWIDGET(wid)
# define WIDGET_OPS(wid)         AGWIDGET_OPS(wid)
# define WIDGET_SUPER_OPS(wid)   AGWIDGET_SUPER_OPS(wid)
# define WSURFACE(wid,id)        AGWIDGET_SURFACE((wid),(id))
# define WTEXTURE(wid,id)        AGWIDGET_TEXTURE((wid),(id))
# define WFONT(wid)              AGWIDGET_FONT(wid)
# define WIDTH(wid)              AGWIDGET(wid)->w
# define HEIGHT(wid)             AGWIDGET(wid)->h
# define WCOLOR(wid,id)          AG_WCOLOR((wid),(id))
# define WCOLOR_DEFAULT(wid,id)  AG_WCOLOR_DEFAULT((wid),(id))
# define WCOLOR_DISABLED(wid,id) AG_WCOLOR_DISABLED((wid),(id))
# define WCOLOR_FOCUSED(wid,id)  AG_WCOLOR_FOCUSED((wid),(id))
# define WCOLOR_HOVER(wid,id)    AG_WCOLOR_HOVER((wid),(id))
# define FG_COLOR                AG_FG_COLOR
# define BG_COLOR                AG_BG_COLOR
# define TEXT_COLOR              AG_TEXT_COLOR
# define LINE_COLOR              AG_LINE_COLOR
# define HIGH_COLOR              AG_HIGH_COLOR
# define LOW_COLOR               AG_LOW_COLOR
# define SELECTION_COLOR         AG_SELECTION_COLOR
#endif /* _AGAR_INTERNAL or _USE_AGAR_GUI */

struct ag_window;
struct ag_font;
AG_TAILQ_HEAD(ag_widgetq, ag_widget);

__BEGIN_DECLS
extern AG_WidgetClass agWidgetClass;
extern const char *_Nullable agStyleAttributes[];
extern const char *_Nullable agWidgetStateNames[];
extern AG_WidgetPalette agDefaultPalette;
#if defined(AG_DEBUG) && defined(AG_WIDGETS)
extern AG_Widget *_Nullable agDebuggerTgt;
#endif
#if defined(AG_WIDGETS)
extern AG_Widget *_Nullable agStyleEditorTgt;
#endif

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

void AG_WidgetShow(void *_Nonnull);
void AG_WidgetHide(void *_Nonnull);
void AG_WidgetShowAll(void *_Nonnull);
void AG_WidgetHideAll(void *_Nonnull);

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
int AG_ExecAction(void *_Nonnull, const AG_Action *_Nonnull);

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
int  ag_widget_enabled(const void *_Nonnull) _Pure_Attribute;
int  ag_widget_disabled(const void *_Nonnull) _Pure_Attribute;
int  ag_widget_visible(const void *_Nonnull) _Pure_Attribute;
int  ag_widget_is_focused(const void *_Nonnull) _Pure_Attribute;
int  ag_widget_is_focused_in_window(const void *_Nonnull) _Pure_Attribute;
int  ag_widget_area(const void *_Nonnull, int,int) _Pure_Attribute;
int  ag_widget_relative_area(const void *_Nonnull, int,int) _Pure_Attribute;
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
# define AG_WidgetIsFocused(o)               ag_widget_is_focused(o)
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
#endif /* !AG_INLINE_WIDGET */

#ifdef AG_LEGACY
# define AG_WIDGET_NOSPACING 0
# define AG_WIDGET_TABLE_EMBEDDABLE 0
# define AG_WidgetInheritSizeRequest  NULL /* is default behavior in 1.6 */
# define AG_WidgetInheritSizeAllocate NULL /* is default behavior in 1.6 */
# define AG_WidgetParentWindow(w) AG_ParentWindow(w)
# define AG_WidgetFocused         AG_WidgetIsFocused(w)
# define AG_WidgetPutPixel32      AG_PutPixel32
# define AG_WidgetPutPixel        AG_PutPixel32
# define AG_DrawPixel             AG_PutPixel32
# define AG_WidgetPutPixelRGB     AG_PutPixelRGB
# define AG_DrawPixelRGB          AG_PutPixelRGB
# define AG_WidgetBlendPixel      AG_BlendPixel32
# define AG_WidgetBlendPixel32    AG_BlendPixel32
# define AG_DrawPixelBlended      AG_BlendPixel32
# define AG_WidgetBlendPixelRGBA  AG_BlendPixelRGBA
# define AG_WidgetShownRecursive  AG_WidgetShowAll
# define AG_WidgetHiddenRecursive AG_WidgetHideAll
void *_Nullable AG_WidgetFind(void *_Nonnull, const char *_Nonnull) DEPRECATED_ATTRIBUTE;
void AG_WidgetInheritDraw(void *_Nonnull) DEPRECATED_ATTRIBUTE;
#endif /* AG_LEGACY */

__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_GUI_WIDGET_H_ */
