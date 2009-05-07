/*	Public domain	*/

#ifndef _AGAR_WIDGET_H_
#define _AGAR_WIDGET_H_

#include <agar/config/have_opengl.h>
#include <agar/config/have_long_double.h>

#include <agar/gui/colors.h>
#include <agar/gui/view.h>
#include <agar/gui/style.h>

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

enum ag_widget_sizespec {
	AG_WIDGET_BAD_SPEC,	/* Parser error */
	AG_WIDGET_PIXELS,	/* Pixel count */
	AG_WIDGET_PERCENT,	/* % of available space */
	AG_WIDGET_STRINGLEN,	/* Width of given string */
	AG_WIDGET_FILL		/* Fill remaining space */
};

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

typedef struct ag_widget {
	struct ag_object obj;

	Uint flags;
#define AG_WIDGET_FOCUSABLE		0x00001 /* Can grab focus */
#define AG_WIDGET_FOCUSED		0x00002 /* Holds focus (optimization) */
#define AG_WIDGET_UNFOCUSED_MOTION	0x00004 /* All mousemotion events */
#define AG_WIDGET_UNFOCUSED_BUTTONUP	0x00008 /* All mousebuttonup events */
#define AG_WIDGET_UNFOCUSED_BUTTONDOWN	0x00010 /* All mousebuttondown events */
#define AG_WIDGET_HFILL			0x00040 /* Expand to fill width */
#define AG_WIDGET_VFILL			0x00080 /* Expand to fill height */
#define AG_WIDGET_HIDE			0x00200 /* Don't draw this widget */
#define AG_WIDGET_DISABLED		0x00400 /* Don't respond to input */
#define AG_WIDGET_CATCH_TAB		0x01000 /* Catch tab key events */
#define AG_WIDGET_PRIO_MOTION		0x02000 /* Block mousemotion events to
						  any other widget, regardless
						  of focus */
#define AG_WIDGET_UNDERSIZE		0x04000 /* Size allocation failed */
#define AG_WIDGET_NOSPACING		0x08000 /* Disable spacings around
						   widget; container-specific */
#define AG_WIDGET_UNFOCUSED_KEYDOWN	0x10000 /* All mousebuttondown events */
#define AG_WIDGET_UNFOCUSED_KEYUP	0x20000 /* All mousebuttondown events */
#define AG_WIDGET_DEBUG_RSENS		0x40000 /* Debug sensitivity rect */
#define AG_WIDGET_EXPAND		(AG_WIDGET_HFILL|AG_WIDGET_VFILL)

	int x, y;			/* Coordinates in container */
	int w, h;			/* Allocated geometry */
	AG_Rect2 rView;			/* Computed view coordinates */
	AG_Rect2 rSens;			/* Rectangle of sensitivity (i.e., to
					   cursor events), in view coords */

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
} AG_Widget;

#define AGWIDGET(wi)		((AG_Widget *)(wi))
#define AGWIDGET_OPS(wi)	((AG_WidgetClass *)OBJECT(wi)->cls)
#define AGWIDGET_SUPER_OPS(wi)	((AG_WidgetClass *)OBJECT(wi)->cls->super)

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

#define AG_WidgetFocused(wi)	(AGWIDGET(wi)->flags&AG_WIDGET_FOCUSED)
#define AG_WidgetDisabled(wi)	(AGWIDGET(wi)->flags&AG_WIDGET_DISABLED)
#define AG_WidgetEnabled(wi)	((AGWIDGET(wi)->flags&AG_WIDGET_DISABLED)==0)

struct ag_window;

__BEGIN_DECLS
extern AG_WidgetClass agWidgetClass;
extern int agKbdDelay;
extern int agKbdRepeat;
extern int agMouseDblclickDelay;
extern int agMouseSpinDelay;
extern int agMouseSpinIval;

void AG_WidgetDraw(void *);
void AG_WidgetSizeReq(void *, AG_SizeReq *);
int  AG_WidgetSizeAlloc(void *, AG_SizeAlloc *);
void AG_WidgetSetFocusable(void *, int);
void AG_WidgetForwardFocus(void *, void *);

void		  AG_WidgetFocus(void *);
void		  AG_WidgetUnfocus(void *);
AG_Widget	 *AG_WidgetFindFocused(void *);
void		 *AG_WidgetFindPoint(const char *, int, int);
void		 *AG_WidgetFindRect(const char *, int, int, int, int);
void		  AG_WidgetUpdateCoords(void *, int, int);
struct ag_window *AG_ParentWindow(void *);
#define		  AG_WidgetParentWindow(w) AG_ParentWindow(w)

int	 AG_WidgetMapSurface(void *, AG_Surface *);
int	 AG_WidgetMapSurfaceNODUP(void *, AG_Surface *);
void	 AG_WidgetReplaceSurface(void *, int, AG_Surface *);
void	 AG_WidgetReplaceSurfaceNODUP(void *, int, AG_Surface *);
void	 AG_WidgetUpdateSurface(void *, int);
#define	 AG_WidgetUnmapSurface(w, n) \
	 AG_WidgetReplaceSurface((w),(n),NULL)
#ifdef HAVE_OPENGL
# define AG_WidgetUpdateSurface(wid,name) do { \
	 AGWIDGET(wid)->surfaceFlags[(name)] |= AG_WIDGET_SURFACE_REGEN; \
} while (0)
#else
# define AG_WidgetUpdateSurface(wid,name)
#endif

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

int   AG_WidgetSensitive(void *, int, int);
void  AG_WidgetMouseMotion(struct ag_window *, AG_Widget *, int, int, int, int, int);
void  AG_WidgetMouseButtonUp(struct ag_window *, AG_Widget *, int, int, int);
int   AG_WidgetMouseButtonDown(struct ag_window *, AG_Widget *, int, int, int);
void  AG_WidgetUnfocusedKeyUp(AG_Widget *, int, int, int);
void  AG_WidgetUnfocusedKeyDown(AG_Widget *, int, int, int);

enum ag_widget_sizespec AG_WidgetParseSizeSpec(const char *, int *);
int			AG_WidgetScrollDelta(Uint32 *);
void			*AG_WidgetFind(AG_Display *, const char *);

void AG_WidgetShownRecursive(void *);
void AG_WidgetHiddenRecursive(void *);

void AG_WidgetInheritDraw(void *);
void AG_WidgetInheritSizeRequest(void *, AG_SizeReq *);
int AG_WidgetInheritSizeAllocate(void *, const AG_SizeAlloc *);

/*
 * Inlines
 */

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

/* Put a single pixel at specified coordinates. */
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

static __inline__ void
AG_WidgetBlendPixel32(void *p, int wx, int wy, Uint32 pixel, AG_BlendFn fn)
{
	Uint8 c[4];

	AG_GetRGBA(pixel, agSurfaceFmt, &c[0],&c[1],&c[2],&c[3]);
	AG_WidgetBlendPixelRGBA(p, wx,wy, c, fn);
}

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
__END_DECLS

#ifdef AG_LEGACY
# include <agar/gui/widget_legacy.h>
#endif /* AG_LEGACY */

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_H_ */
