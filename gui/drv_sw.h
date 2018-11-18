/*	Public domain	*/

/*
 * Single-window / framebuffer graphics driver interface.
 */

typedef struct ag_driver_sw_class {
	struct ag_driver_class _inherit;
	Uint flags;

	/* Create or attach to a graphics context */
	int  (*_Nonnull openVideo)(void *_Nonnull, Uint,Uint, int, Uint);
	int  (*_Nonnull openVideoContext)(void *_Nonnull, void *_Nonnull, Uint);
	int  (*_Nonnull setVideoContext)(void *_Nonnull, void *_Nonnull);
	void (*_Nonnull closeVideo)(void *_Nonnull);

	/* Resize the display */
	int (*_Nonnull videoResize)(void *_Nonnull, Uint,Uint);

	/* Capture display contents to a new software surface */
	AG_Surface *_Nullable (*_Nonnull videoCapture)(void *_Nonnull);

	/* Clear the background */
	void (*_Nonnull videoClear)(void *_Nonnull, AG_Color);
} AG_DriverSwClass;

struct ag_style;

/* General window alignment in view */
enum ag_window_alignment {
	AG_WINDOW_ALIGNMENT_NONE,
	AG_WINDOW_TL,
	AG_WINDOW_TC,
	AG_WINDOW_TR,
	AG_WINDOW_ML,
	AG_WINDOW_MC,
	AG_WINDOW_MR,
	AG_WINDOW_BL,
	AG_WINDOW_BC,
	AG_WINDOW_BR,
	AG_WINDOW_ALIGNMENT_LAST
};

/* Window manager operation */
enum ag_wm_operation {
	AG_WINOP_NONE,		/* No operation */
	AG_WINOP_MOVE,		/* Move window */
	AG_WINOP_LRESIZE,	/* Resize (via left control) */
	AG_WINOP_RRESIZE,	/* Resize (via right control) */
	AG_WINOP_HRESIZE	/* Resize (via horizontal control) */
};

/* Single-window driver instance */
typedef struct ag_driver_sw {
	struct ag_driver _inherit;
	Uint w, h, depth;		/* Video resolution */
	Uint flags;
#define AG_DRIVER_SW_OVERLAY	0x01	/* "Overlay" mode */
#define AG_DRIVER_SW_BGPOPUP	0x02	/* Enable generic background popup */
#define AG_DRIVER_SW_FULLSCREEN	0x04	/* Currently in full-screen mode */
#define AG_DRIVER_SW_REDRAW	0x08	/* Global redraw request */

	struct ag_window *_Nullable winSelected;    /* Window being manipulated */
	struct ag_window *_Nullable winLastKeydown; /* For keyboard processing */

	AG_List *_Nonnull Lmodal;	/* Modal window stack */

	enum ag_wm_operation winop;	/* WM operation in progress */
	int windowXOutLimit;		/* Limit past left/right boundary */
	int windowBotOutLimit;		/* Limit past bottom boundary */
	int windowIconWidth;		/* Preferred window icon dimensions */
	int windowIconHeight;
	Uint rNom;			/* Nominal refresh rate (ms) */
	int rCur;			/* Effective refresh rate (ms) */
	AG_Color bgColor;		/* "bgColor" setting */
	Uint rLast;			/* Refresh rate timestamp */
} AG_DriverSw;

#define AGDRIVER_SW(obj) ((AG_DriverSw *)(obj))
#define AGDRIVER_SW_CLASS(obj) ((struct ag_driver_sw_class *)(AGOBJECT(obj)->cls))

__BEGIN_DECLS
extern AG_ObjectClass agDriverSwClass;

extern AG_DriverSw *_Nullable agDriverSw;	/* Root driver instance */

struct ag_size_alloc;

void AG_WM_BackgroundPopupMenu(AG_DriverSw *_Nonnull);
void AG_WM_CommitWindowFocus(struct ag_window *_Nonnull);

int  AG_ResizeDisplay(int,int);
void AG_PostResizeDisplay(AG_DriverSw *_Nonnull);
void AG_SetVideoResizeCallback(void (*_Nullable)(Uint,Uint));

void AG_WM_LimitWindowToView(struct ag_window *_Nonnull);
void AG_WM_LimitWindowToDisplaySize(AG_Driver *_Nonnull,
                                    struct ag_size_alloc *_Nonnull);
void AG_WM_GetPrefPosition(struct ag_window *_Nonnull,
                           int *_Nonnull,int *_Nonnull, int,int);

void AG_WM_MoveBegin(struct ag_window *_Nonnull);
void AG_WM_MoveEnd(struct ag_window *_Nonnull);
void AG_WM_MouseMotion(AG_DriverSw *_Nonnull, struct ag_window *_Nonnull,
                       int,int);

/* Blank the display background. */
static __inline__ void
AG_ClearBackground(void)
{
	if (agDriverSw != NULL) {
		AG_Color c = { 0,0,0,0 };
		AGDRIVER_SW_CLASS(agDriverSw)->videoClear(agDriverSw, c);
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
	return agDriverOps->setRefreshRate(agDriverSw, fps);
}

/* Evaluate whether there are pending events to be processed. */
static __inline__ int
AG_PendingEvents(AG_Driver *_Nullable drv)
{
	if (drv != NULL) {
		return AGDRIVER_CLASS(drv)->pendingEvents(drv);
	} else {
		return agDriverOps->pendingEvents(agDriverSw);
	}
}

/* Retrieve the next pending event, translated to generic AG_DriverEvent form. */
static __inline__ int
AG_GetNextEvent(AG_Driver *_Nullable drv, AG_DriverEvent *_Nonnull dev)
{
	if (drv != NULL) {
		return AGDRIVER_CLASS(drv)->getNextEvent(drv, dev);
	} else {
		return agDriverOps->getNextEvent(agDriverSw, dev);
	}
}

/* Process the next pending event in generic manner. */
static __inline__ int
AG_ProcessEvent(AG_Driver *_Nullable drv, AG_DriverEvent *_Nonnull dev)
{
	if (drv != NULL) {
		return AGDRIVER_CLASS(drv)->processEvent(drv, dev);
	} else {
		return agDriverOps->processEvent(agDriverSw, dev);
	}
}
__END_DECLS
