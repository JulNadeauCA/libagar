/*	Public domain	*/

/*
 * Single-window graphics driver framework. In this mode, the driver
 * creates a single "native" display context and Agar emulates a window
 * manager internally.
 */

typedef struct ag_driver_sw_class {
	struct ag_driver_class _inherit;
	Uint flags;
	/* Create or attach to a graphics display */
	int  (*openVideo)(void *drv, Uint w, Uint h, int depth, Uint flags);
	int  (*openVideoContext)(void *drv, void *ctx, Uint flags);
	void (*closeVideo)(void *drv);
	/* Resize the display */
	int  (*videoResize)(void *drv, Uint w, Uint h);
	/* Capture display contents to surface */
	int  (*videoCapture)(void *drv, AG_Surface **);
	/* Clear background */
	void (*videoClear)(void *drv, AG_Color c);
} AG_DriverSwClass;

struct ag_style;

enum ag_wm_operation {
	AG_WINOP_NONE,		/* No operation */
	AG_WINOP_MOVE,		/* Move window */
	AG_WINOP_LRESIZE,	/* Resize (via left control) */
	AG_WINOP_RRESIZE,	/* Resize (via right control) */
	AG_WINOP_HRESIZE	/* Resize (via horizontal control) */
};

typedef struct ag_driver_sw {
	struct ag_driver _inherit;
	Uint w, h, depth;		/* Video resolution */
	Uint flags;
#define AG_DRIVER_SW_OVERLAY	0x01	/* "Overlay" mode */
#define AG_DRIVER_SW_BGPOPUP	0x02	/* Enable generic background popup */

	struct ag_window *winToFocus;	/* Request focus */
	struct ag_window *winSelected;	/* Window being moved/resized/etc */
	struct ag_window *winLastKeydown; /* For keyboard processing */
	AG_List *Lmodal;		/* Modal window stack */
	enum ag_wm_operation winop;	/* WM operation in progress */
	struct ag_style *style;		/* Default style for new windows */
} AG_DriverSw;

#define AGDRIVER_SW(obj) ((AG_DriverSw *)(obj))
#define AGDRIVER_SW_CLASS(obj) ((struct ag_driver_sw_class *)(AGOBJECT(obj)->cls))

__BEGIN_DECLS
extern AG_ObjectClass    agDriverSwClass;
extern AG_DriverSw      *agDriverSw;

struct ag_size_alloc;

void AG_WM_BackgroundPopupMenu(AG_DriverSw *);
int  AG_ResizeDisplay(int, int);
void AG_SetVideoResizeCallback(void (*)(Uint, Uint));
void AG_ChangeWindowFocus(void);
void AG_WM_LimitWindowToView(struct ag_window *);
void AG_WM_LimitWindowToDisplaySize(AG_Driver *, struct ag_size_alloc *);

void AG_WM_MouseMotion(AG_DriverSw *, struct ag_window *, int, int);

/* Blank the display background. */
static __inline__ void
AG_ClearBackground(void)
{
	if (AGDRIVER_SINGLE(agDriver)) {
		AG_Color c = { 0,0,0,0 };
		AGDRIVER_SW_CLASS(agDriver)->videoClear(agDriver, c);
	}
}

/* Configure the display refresh rate (driver-dependent). */
static __inline__ int
AG_SetRefreshRate(int fps)
{
	if (agDriverOps->setRefreshRate == NULL) {
		AG_SetError("Refresh rate not applicable to graphics driver");
		return (-1);
	}
	return agDriverOps->setRefreshRate(agDriver, fps);
}
__END_DECLS
