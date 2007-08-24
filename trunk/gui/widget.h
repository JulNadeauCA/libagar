/*	Public domain	*/

#ifndef _AGAR_WIDGET_H_
#define _AGAR_WIDGET_H_

#ifdef _AGAR_INTERNAL
#include <config/have_opengl.h>
#include <gui/style.h>
#include <gui/colors.h>
#include <core/view.h>
#else
#include <agar/config/have_opengl.h>
#include <agar/gui/style.h>
#include <agar/gui/colors.h>
#include <agar/core/view.h>
#endif

#include "begin_code.h"

#define AG_WIDGET_BINDING_NAME_MAX	16

typedef struct ag_size_req {
	int w, h;			/* Requested geometry in pixels */
} AG_SizeReq;

typedef struct ag_size_alloc {
	int w, h;			/* Allocated geometry in pixels */
	int x, y;			/* Allocated position in pixels */
} AG_SizeAlloc;

typedef struct ag_widget_ops {
	const AG_ObjectOps ops;
	void (*draw)(void *);
	void (*size_request)(void *, AG_SizeReq *);
	int  (*size_allocate)(void *, const AG_SizeAlloc *);
} AG_WidgetOps;

typedef enum ag_widget_binding_type {
	AG_WIDGET_NONE,
	AG_WIDGET_BOOL,
	AG_WIDGET_UINT,
	AG_WIDGET_INT,
	AG_WIDGET_UINT8,
	AG_WIDGET_SINT8,
	AG_WIDGET_UINT16,
	AG_WIDGET_SINT16,
	AG_WIDGET_UINT32,
	AG_WIDGET_SINT32,
	AG_WIDGET_UINT64,
	AG_WIDGET_SINT64,
	AG_WIDGET_FLOAT,
	AG_WIDGET_DOUBLE,
	AG_WIDGET_STRING,
	AG_WIDGET_POINTER,
	AG_WIDGET_PROP,
	AG_WIDGET_FLAG,
	AG_WIDGET_FLAG8,
	AG_WIDGET_FLAG16,
	AG_WIDGET_FLAG32
} AG_WidgetBindingType;

enum ag_widget_sizespec {
	AG_WIDGET_BAD_SPEC,
	AG_WIDGET_PIXELS,			/* Pixel count */
	AG_WIDGET_PERCENT,			/* % of available space */
	AG_WIDGET_STRINGLEN,			/* Width of given string */
	AG_WIDGET_FILL				/* Fill remaining space */
};

typedef struct ag_flag_descr {
	Uint bitmask;			/* Bitmask */
	const char *descr;		/* Bit(s) description */
	int writeable;			/* User-editable */
} AG_FlagDescr;

typedef struct ag_widget_binding {
	char name[AG_WIDGET_BINDING_NAME_MAX];	/* Binding identifier */
	int type;				/* Real binding type */
	int vtype;				/* Virtual binding type
						   (translated PROP types) */
	AG_Mutex *mutex;			/* Mutex to acquire or NULL */
	void *p1;				/* Pointer to variable */
	union {
		char prop[AG_PROP_KEY_MAX];	/* Property name (for PROP) */
		size_t size;			/* Size (for STRING) */
		Uint32 bitmask;			/* Bitmask (for FLAG) */
	} data;
	SLIST_ENTRY(ag_widget_binding) bindings;
} AG_WidgetBinding;

struct ag_popup_menu;

typedef struct ag_widget {
	struct ag_object obj;

	int flags;
#define AG_WIDGET_FOCUSABLE		0x0001 /* Can grab focus */
#define AG_WIDGET_FOCUSED		0x0002 /* Holds focus (optimization) */
#define AG_WIDGET_UNFOCUSED_MOTION	0x0004 /* All mousemotion events */
#define AG_WIDGET_UNFOCUSED_BUTTONUP	0x0008 /* All mousebuttonup events */
#define AG_WIDGET_UNFOCUSED_BUTTONDOWN	0x0010 /* All mousebuttondown events */
#define AG_WIDGET_CLIPPING		0x0020 /* Automatic clipping */
#define AG_WIDGET_HFILL			0x0040 /* Expand to fill width */
#define AG_WIDGET_VFILL			0x0080 /* Expand to fill height */
#define AG_WIDGET_EXCEDENT		0x0100 /* Used internally for scaling */
#define AG_WIDGET_HIDE			0x0200 /* Don't draw this widget */
#define AG_WIDGET_DISABLED		0x0400 /* Don't respond to input */
#define AG_WIDGET_STATIC		0x0800 /* Use the redraw flag method */
#define AG_WIDGET_FOCUS_PARENT_WIN	0x1000 /* Focus parent win on focus */
#define AG_WIDGET_PRIO_MOTION		0x2000 /* Block mousemotion events to
						  any other widget, regardless
						  of focus */
#define AG_WIDGET_UNDERSIZE		0x4000 /* Size allocation failed */
#define AG_WIDGET_IGNORE_PADDING	0x8000 /* Ignore container padding
					          (container-specific) */
#define AG_WIDGET_EXPAND		(AG_WIDGET_HFILL|AG_WIDGET_VFILL)

	int redraw;			/* Redraw flag (for WIDGET_STATIC) */
	int cx, cy, cx2, cy2;		/* Cached view coords (optimization) */
	int x, y;			/* Coordinates in container */
	int w, h;			/* Allocated geometry */
	SDL_Rect rClipSave;		/* Saved clipping rectangle */
	const AG_WidgetStyleMod *style;	/* Style mods (inherited from parent) */

	SDL_Surface **surfaces;		/* Registered surfaces */
	Uint *surfaceFlags;		/* Surface flags */
#define AG_WIDGET_SURFACE_NODUP	0x01	/* Don't free on destroy */
	Uint nsurfaces;
#ifdef HAVE_OPENGL
	Uint *textures;			/* Cached OpenGL textures */
	float *texcoords;		/* Cached texture coordinates */
#endif

	AG_Mutex bindings_lock;			 /* Lock on all bindings */
	SLIST_HEAD(,ag_widget_binding) bindings; /* List of variable bindings */
	SLIST_HEAD(,ag_popup_menu) menus;	 /* Managed menus */
} AG_Widget;

#define AGWIDGET(wi)			((AG_Widget *)(wi))
#define AGWIDGET_OPS(wi)		((AG_WidgetOps *)OBJECT(wi)->ops)
#define AGWIDGET_SURFACE(wi, ind)	AGWIDGET(wi)->surfaces[ind]
#define AGWIDGET_TEXTURE(wi, ind)	AGWIDGET(wi)->textures[ind]
#define AGWIDGET_TEXCOORD(wi, ind)	AGWIDGET(wi)->texcoords[(ind)*4]
#define AGWIDGET_SURFACE_NODUP(wi, ind)	(AGWIDGET(wi)->surfaceFlags[ind] & \
					 AG_WIDGET_SURFACE_NODUP)

#ifdef _AGAR_INTERNAL
#define WIDGET(wi)			AGWIDGET(wi)
#define WIDGET_OPS(wi)			AGWIDGET_OPS(wi)
#define WSURFACE(wi,ind)		AGWIDGET_SURFACE((wi),(ind))
#define WTEXTURE(wi,ind)		AGWIDGET_TEXTURE((wi),(ind))
#define WTEXCOORD(wi,ind)		AGWIDGET_TEXCOORD((wi),(ind))
#define WSURFACE_NODUP(wi,ind)		AGWIDGET_SURFACE_NODUP((wi),(ind))
#endif /* _AGAR_INTERNAL */

#define AG_WidgetFocused(wi)	(AGWIDGET(wi)->flags&AG_WIDGET_FOCUSED)
#define AG_WidgetDisabled(wi)	(AGWIDGET(wi)->flags&AG_WIDGET_DISABLED)
#define AG_WidgetEnabled(wi)	((AGWIDGET(wi)->flags&AG_WIDGET_DISABLED)==0)

#ifdef DEBUG
#define AG_WidgetRedraw(wi)						\
	if (((wi)->flags & AG_WIDGET_STATIC) == 0)			\
		fatal("AG_WidgetRedraw() called on non-static widget"); \
	AGWIDGET(wi)->redraw++
#else
#define AG_WidgetRedraw(wi) AGWIDGET(wi)->redraw++
#endif

struct ag_window;

__BEGIN_DECLS
extern const AG_WidgetOps agWidgetOps;
extern int agKbdDelay;
extern int agKbdRepeat;
extern int agMouseDblclickDelay;
extern int agMouseSpinDelay;
extern int agMouseSpinIval;

AG_Widget *AG_WidgetNew(void *, Uint);
void	   AG_WidgetInit(void *, const void *, Uint);
void	   AG_WidgetDestroy(void *);
void	   AG_WidgetDraw(void *);

void	   AG_WidgetSizeReq(void *, AG_SizeReq *);
int	   AG_WidgetSizeAlloc(void *, AG_SizeAlloc *);

__inline__ void	   AG_WidgetSetFocusable(void *, int);
__inline__ void	   AG_WidgetEnable(void *);
__inline__ void	   AG_WidgetDisable(void *);

void		*AG_WidgetFind(AG_Display *, const char *);
void		 AG_WidgetFocus(void *);
void		 AG_WidgetUnfocus(void *);
AG_Widget	*AG_WidgetFindFocused(void *);
__inline__ int	 AG_WidgetRelativeArea(void *, int, int);
__inline__ int	 AG_WidgetArea(void *, int, int);
void		 AG_WidgetUpdateCoords(void *, int, int);
struct ag_window *AG_WidgetParentWindow(void *);

int		 AG_WidgetMapSurface(void *, SDL_Surface *);
int		 AG_WidgetMapSurfaceNODUP(void *, SDL_Surface *);
__inline__ void	 AG_WidgetReplaceSurface(void *, int, SDL_Surface *);
__inline__ void	 AG_WidgetReplaceSurfaceNODUP(void *, int, SDL_Surface *);
__inline__ void	 AG_WidgetUpdateSurface(void *, int);
#define		 AG_WidgetUnmapSurface(w, n) \
		 AG_WidgetReplaceSurface((w),(n),NULL)

void	 AG_WidgetBlit(void *, SDL_Surface *, int, int);
void	 AG_WidgetBlitFrom(void *, void *, int, SDL_Rect *, int, int);
#define  AG_WidgetBlitSurface(p,n,x,y) \
	 AG_WidgetBlitFrom((p),(p),(n),NULL,(x),(y))
void	 AG_WidgetBlitSurfaceGL(void *, int, float, float);
void	 AG_WidgetPushClipRect(void *, int, int, Uint, Uint);
void	 AG_WidgetPopClipRect(void *);
int	 AG_WidgetIsOcculted(AG_Widget *);

__inline__ void	 AG_SetCursor(int);
__inline__ void	 AG_UnsetCursor(void);

#define		AG_WidgetPutPixel AG_WidgetPutPixel32
__inline__ void AG_WidgetPutPixel32(void *, int, int, Uint32);
__inline__ void AG_WidgetPutPixel32OrClip(void *, int, int, Uint32);
__inline__ void AG_WidgetPutPixelRGB(void *, int, int, Uint8, Uint8, Uint8);
__inline__ void AG_WidgetPutPixelRGBOrClip(void *, int, int, Uint8, Uint8,
		                           Uint8);

#define		AG_WidgetBlendPixel AG_WidgetBlendPixelRGBA
__inline__ void AG_WidgetBlendPixelRGBA(void *, int, int, Uint8 [4],
		                        enum ag_blend_func);
__inline__ void AG_WidgetBlendPixel32(void *, int, int, Uint32, AG_BlendFn);

void  AG_WidgetMouseMotion(struct ag_window *, AG_Widget *, int, int, int,
	                   int, int);
void  AG_WidgetMouseButtonUp(struct ag_window *, AG_Widget *, int, int, int);
int   AG_WidgetMouseButtonDown(struct ag_window *, AG_Widget *, int, int, int);

AG_WidgetBinding *AG_WidgetBind(void *, const char *,
	                        AG_WidgetBindingType, ...);
AG_WidgetBinding *AG_WidgetBindMp(void *, const char *, AG_Mutex *,
			          AG_WidgetBindingType, ...);
AG_WidgetBinding *AG_WidgetGetBinding(void *, const char *, ...);
__inline__ void	  AG_WidgetLockBinding(AG_WidgetBinding *);
__inline__ void	  AG_WidgetUnlockBinding(AG_WidgetBinding *);
__inline__ void	  AG_WidgetBindingChanged(AG_WidgetBinding *);
__inline__ int	  AG_WidgetCopyBinding(void *, const char *, void *,
		                       const char *);

#define AG_WidgetBindBool(w,b,p) AG_WidgetBind((w),(b),AG_WIDGET_BOOL,(p))
#define AG_WidgetBindInt(w,b,p) AG_WidgetBind((w),(b),AG_WIDGET_INT,(p))
#define AG_WidgetBindUint(w,b,p) AG_WidgetBind((w),(b),AG_WIDGET_UINT,(p))
#define AG_WidgetBindUint8(w,b,p) AG_WidgetBind((w),(b),AG_WIDGET_UINT8,(p))
#define AG_WidgetBindSint8(w,b,p) AG_WidgetBind((w),(b),AG_WIDGET_SINT8,(p))
#define AG_WidgetBindUint16(w,b,p) AG_WidgetBind((w),(b),AG_WIDGET_UINT16,(p))
#define AG_WidgetBindSint16(w,b,p) AG_WidgetBind((w),(b),AG_WIDGET_SINT16,(p))
#define AG_WidgetBindUint32(w,b,p) AG_WidgetBind((w),(b),AG_WIDGET_UINT32,(p))
#define AG_WidgetBindSint32(w,b,p) AG_WidgetBind((w),(b),AG_WIDGET_SINT32,(p))
#define AG_WidgetBindUint64(w,b,p) AG_WidgetBind((w),(b),AG_WIDGET_UINT64,(p))
#define AG_WidgetBindSint64(w,b,p) AG_WidgetBind((w),(b),AG_WIDGET_SINT64,(p))
#define AG_WidgetBindFloat(w,b,p) AG_WidgetBind((w),(b),AG_WIDGET_FLOAT,(p))
#define AG_WidgetBindDouble(w,b,p) AG_WidgetBind((w),(b),AG_WIDGET_DOUBLE,(p))
#define AG_WidgetBindPointer(w,b,p) AG_WidgetBind((w),(b),AG_WIDGET_POINTER,(p))
#define AG_WidgetBindProp(w,b,o,k) AG_WidgetBind((w),(b),AG_WIDGET_PROP,(o),(k))
#define AG_WidgetBindString(w,b,p,len) AG_WidgetBind((w),(b),AG_WIDGET_STRING,\
				       (p),(len))
#define AG_WidgetBindFlag(w,b,p,mask) AG_WidgetBind((w),(b),AG_WIDGET_FLAG,\
                                      (p),(mask))
#define AG_WidgetBindFlag8(w,b,p,mask) AG_WidgetBind((w),(b),AG_WIDGET_FLAG8,\
                                       (p),(mask))
#define AG_WidgetBindFlag16(w,b,p,mask) AG_WidgetBind((w),(b),AG_WIDGET_FLAG16,\
                                        (p),(mask))
#define AG_WidgetBindFlag32(w,b,p,mask) AG_WidgetBind((w),(b),AG_WIDGET_FLAG32,\
                                        (p),(mask))

__inline__ Uint	 	 AG_WidgetUint(void *, const char *);
__inline__ int		 AG_WidgetInt(void *, const char *);
#define			 AG_WidgetBool AG_WidgetInt
__inline__ Uint8	 AG_WidgetUint8(void *, const char *);
__inline__ Sint8	 AG_WidgetSint8(void *, const char *);
__inline__ Uint16	 AG_WidgetUint16(void *, const char *);
__inline__ Sint16	 AG_WidgetSint16(void *, const char *);
__inline__ Uint32	 AG_WidgetUint32(void *, const char *);
__inline__ Sint32	 AG_WidgetSint32(void *, const char *);
__inline__ Uint64	 AG_WidgetUint64(void *, const char *);
__inline__ Sint64	 AG_WidgetSint64(void *, const char *);
__inline__ float	 AG_WidgetFloat(void *, const char *);
__inline__ double	 AG_WidgetDouble(void *, const char *);

__inline__ void	  *AG_WidgetPointer(void *, const char *);
__inline__ char	  *AG_WidgetString(void *, const char *);
__inline__ size_t  AG_WidgetCopyString(void *, const char *, char *, size_t)
		       BOUNDED_ATTRIBUTE(__string__, 3, 4);

__inline__ void	 AG_WidgetSetUint(void *, const char *, Uint);
__inline__ void	 AG_WidgetSetInt(void *, const char *, int);
#define		 AG_WidgetSetBool AG_WidgetSetInt
__inline__ void	 AG_WidgetSetUint8(void *, const char *, Uint8);
__inline__ void	 AG_WidgetSetSint8(void *, const char *, Sint8);
__inline__ void	 AG_WidgetSetUint16(void *, const char *, Uint16);
__inline__ void	 AG_WidgetSetSint16(void *, const char *, Sint16);
__inline__ void	 AG_WidgetSetUint32(void *, const char *, Uint32);
__inline__ void	 AG_WidgetSetSint32(void *, const char *, Sint32);
__inline__ void	 AG_WidgetSetUint64(void *, const char *, Uint64);
__inline__ void	 AG_WidgetSetSint64(void *, const char *, Sint64);
__inline__ void	 AG_WidgetSetFloat(void *, const char *, float);
__inline__ void	 AG_WidgetSetDouble(void *, const char *, double);
__inline__ void	 AG_WidgetSetString(void *, const char *, const char *);
__inline__ void	 AG_WidgetSetPointer(void *, const char *, void *);

enum ag_widget_sizespec AG_WidgetParseSizeSpec(const char *, int *);
__inline__ int AG_WidgetScrollDelta(Uint32 *);

void AG_WidgetShownRecursive(void *);
void AG_WidgetHiddenRecursive(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_H_ */
