/*	Public domain	*/

/*
 * Multiple-window graphics driver framework. In this mode, Agar offers an
 * interface to an existing window manager, as opposed to providing one
 * internally (i.e., each Agar window corresponds to a "native" window).
 */

struct ag_size_alloc;

typedef struct ag_driver_mw_class {
	struct ag_driver_class _inherit;

	/* Open/close native windows */
	int  (*openWindow)(struct ag_window *, AG_Rect r, int bpp, Uint flags);
	void (*closeWindow)(struct ag_window *);

	/* Show and hide window */
	int (*mapWindow)(struct ag_window *);
	int (*unmapWindow)(struct ag_window *);

	/* Configure stacking order and parenting */
	int (*raiseWindow)(struct ag_window *);
	int (*lowerWindow)(struct ag_window *);
	int (*reparentWindow)(struct ag_window *, struct ag_window *, int, int);

	/* Change and query input focus state */
	int (*getInputFocus)(struct ag_window **);
	int (*setInputFocus)(struct ag_window *);

	/* Move and resize windows */
	int  (*moveWindow)(struct ag_window *, int x, int y);
	int  (*resizeWindow)(struct ag_window *, Uint w, Uint h);
	int  (*moveResizeWindow)(struct ag_window *, struct ag_size_alloc *a);
	void (*preResizeCallback)(struct ag_window *);
	void (*postResizeCallback)(struct ag_window *, struct ag_size_alloc *a);

	/* Capture window framebuffer contents (unlike renderToSurface) */
	int (*captureWindow)(struct ag_window *, AG_Surface **s);
	
	/* Configure window parameters */
	int  (*setBorderWidth)(struct ag_window *, Uint w);
	int  (*setWindowCaption)(struct ag_window *, const char *);
	void (*setTransientFor)(struct ag_window *, struct ag_window *);
	int  (*setOpacity)(struct ag_window *, float opacity);
	void (*tweakAlignment)(struct ag_window *, struct ag_size_alloc *a,
	                       Uint wMax, Uint hMax);
} AG_DriverMwClass;

typedef struct ag_driver_mw {
	struct ag_driver _inherit;
	Uint flags;
#define AG_DRIVER_MW_OPEN	0x01		/* Rendering is operational */
	struct ag_window *win;			/* Back pointer to window */
} AG_DriverMw;

#define AGDRIVER_MW(obj) ((AG_DriverMw *)(obj))
#define AGDRIVER_MW_CLASS(obj) ((struct ag_driver_mw_class *)(AGOBJECT(obj)->cls))

/* Flags to openWindow */
#define AG_DRIVER_MW_ANYPOS		0x01	/* Autoposition window */
#define AG_DRIVER_MW_ANYPOS_AVAIL	0x02	/* Autopositioning supported */

__BEGIN_DECLS
extern AG_ObjectClass  agDriverMwClass;
extern AG_DriverMw    *agDriverMw;		/* Root driver instance */
extern AG_List        *agModalWindows;		/* Modal window stack */
extern int             agModalWindowsRefs;
__END_DECLS
