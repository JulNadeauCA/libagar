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
	int  (*_Nonnull openWindow)(struct ag_window *_Nonnull,
	                            const AG_Rect *_Nonnull,
	                            int, Uint);
	void (*_Nonnull closeWindow)(struct ag_window *_Nonnull);

	/* Show and hide window */
	int (*_Nonnull mapWindow)(struct ag_window *_Nonnull);
	int (*_Nonnull unmapWindow)(struct ag_window *_Nonnull);

	/* Configure stacking order and parenting */
	int (*_Nonnull raiseWindow)(struct ag_window *_Nonnull);
	int (*_Nonnull lowerWindow)(struct ag_window *_Nonnull);
	int (*_Nonnull reparentWindow)(struct ag_window *_Nonnull,
	                               struct ag_window *_Nonnull, int,int);

	/* Change and query input focus state */
	int (*_Nonnull getInputFocus)(struct ag_window *_Nonnull *_Nullable);
	int (*_Nonnull setInputFocus)(struct ag_window *_Nonnull);

	/* Move and resize windows */
	int  (*_Nonnull moveWindow)(struct ag_window *_Nonnull, int, int);
	int  (*_Nonnull resizeWindow)(struct ag_window *_Nonnull, Uint, Uint);
	int  (*_Nonnull moveResizeWindow)(struct ag_window *_Nonnull,
	                                  struct ag_size_alloc *_Nonnull);
	void (*_Nonnull preResizeCallback)(struct ag_window *_Nonnull);
	void (*_Nonnull postResizeCallback)(struct ag_window *_Nonnull,
	                                    struct ag_size_alloc *_Nonnull);

	/* Configure window parameters */
	int  (*_Nullable setBorderWidth)(struct ag_window *_Nonnull, Uint);
	int  (*_Nullable setWindowCaption)(struct ag_window *_Nonnull,
	                                   const char *_Nonnull);
	void (*_Nullable setTransientFor)(struct ag_window *_Nonnull,
	                                  struct ag_window *_Nullable);
	int  (*_Nullable setOpacity)(struct ag_window *_Nonnull, float);
	void (*_Nullable tweakAlignment)(struct ag_window *_Nonnull,
	                                 struct ag_size_alloc *_Nonnull,
	                                 Uint,Uint);
	/* Set window size bounds */
	void (*_Nullable setWindowMinSize)(struct ag_window *_Nonnull, int,int);
	void (*_Nullable setWindowMaxSize)(struct ag_window *_Nonnull, int,int);
} AG_DriverMwClass;

typedef struct ag_driver_mw {
	struct ag_driver _inherit;
	struct ag_window *_Nullable win;	/* Back pointer to window */
	Uint flags;
#define AG_DRIVER_MW_OPEN	0x01		/* Rendering is operational */
	Uint32 windowID;			/* Optional window ID */
} AG_DriverMw;

#define  AGDRIVERMW(o)       ((AG_DriverMw *)(o))
#define AGcDRIVERMW(o)       ((const AG_DriverMw *)(o))
#define  AG_DRIVER_MW_ISA(o) (((AGOBJECT(o)->cid & 0xffff0000) >> 16) == 0x0502)
#define  AG_DRIVER_MW_SELF()    AGDRIVERMW(  AG_OBJECT(0,        "AG_Driver:AG_DriverMw:*") )
#define  AG_DRIVER_MW_PTR(n)    AGDRIVERMW(  AG_OBJECT((n),      "AG_Driver:AG_DriverMw:*") )
#define  AG_DRIVER_MW_NAMED(n)  AGDRIVERMW(  AG_OBJECT_NAMED((n),"AG_Driver:AG_DriverMw:*") )
#define AG_cDRIVER_MW_SELF()   AGcDRIVERMW( AG_cOBJECT(0,        "AG_Driver:AG_DriverMw:*") )
#define AG_cDRIVER_MW_PTR(n)   AGcDRIVERMW( AG_cOBJECT((n),      "AG_Driver:AG_DriverMw:*") )
#define AG_cDRIVER_MW_NAMED(n) AGcDRIVERMW( AG_cOBJECT_NAMED((n),"AG_Driver:AG_DriverMw:*") )

#define AGDRIVER_MW_CLASS(o) ((struct ag_driver_mw_class *)(AGOBJECT(o)->cls))

/* Flags to openWindow */
#define AG_DRIVER_MW_ANYPOS		0x01	/* Autoposition window */
#define AG_DRIVER_MW_ANYPOS_AVAIL	0x02	/* Autopositioning supported */

__BEGIN_DECLS
extern AG_ObjectClass agDriverMwClass;
extern AG_DriverMw *_Nullable agDriverMw;	/* Root driver instance */
__END_DECLS
