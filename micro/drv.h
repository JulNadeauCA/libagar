/*	Public domain	*/
/*
 * Micro-Agar Driver Framework.
 */

#ifndef _AGAR_MICRO_DRV_H_
#define _AGAR_MICRO_DRV_H_

#include <agar/micro/surface.h>
#include <agar/micro/begin.h>

struct ma_widget;
struct ma_window;

/* Generic graphics driver class */
typedef struct ma_driver_class {
	struct ag_object_class _inherit; /* [AG_Object] -> [MA_Driver] */
	const char *_Nonnull name;	 /* Short name */

	Uint16 flags;			/* [ Capabilities ] */
#define MA_DRIVER_SPRITES   0x0001	/* Hardware sprites */
#define MA_DRIVER_SPBG_COLL 0x0002	/* Sprite<->BG collision detection */
#define MA_DRIVER_SPSP_COLL 0x0004	/* Sprite<->Sprite collision detection */
#define MA_DRIVER_SOUND     0x0008	/* Audio device */
#define MA_DRIVER_MOUSE     0x0010	/* Mouse interface */
#define MA_DRIVER_JOYSTICK  0x0020	/* Joystick interface */
#define MA_DRIVER_CASSETTE  0x0040	/* Cassette controls */
#define MA_DRIVER_TOD_CLOCK 0x0080	/* Time of Day clock */
#define MA_DRIVER_TIMER_A   0x0100	/* Hardware timer A */
#define MA_DRIVER_TIMER_B   0x0200	/* Hardware timer B */

	/* Initialization */
	Sint8 (*_Nonnull open)(const char *_Nullable);
	void  (*_Nonnull close)(void);
	void  (*_Nonnull getDisplaySize)(Uint16 *_Nonnull, Uint16 *_Nonnull);
	
	/* Rendering Ops */
	void (*_Nonnull beginRendering)(void);
	void (*_Nonnull renderWindow)(struct ma_window *_Nonnull);
	void (*_Nonnull endRendering)(void);

	void (*_Nonnull blitSurface)(struct ma_widget *_Nonnull,
	                             const MA_Surface *_Nonnull, Uint8,Uint8);

	void (*_Nonnull blitSurfaceFrom)(struct ma_widget *_Nonnull,
	                                 Uint8, const MA_Rect *_Nullable,
					 Uint8,Uint8);

	/* GUI Rendering Primitives */
	void (*_Nonnull fillRect)(struct ma_widget *_Nonnull,
	                          const MA_Rect *_Nonnull,
	                          const MA_Color *_Nonnull);
	void (*_Nonnull putPixel)(struct ma_widget *_Nonnull, Uint8,Uint8,
	                          const MA_Color *_Nonnull);
	void (*_Nonnull putPixel8)(struct ma_widget *_Nonnull, Uint8,Uint8, Uint8);
	void (*_Nonnull blendPixel)(struct ma_widget *_Nonnull, Uint8,Uint8,
	                            const MA_Color *_Nonnull);
	void (*_Nonnull drawLine)(struct ma_widget *_Nonnull, Uint8,Uint8,
	                          Uint8,Uint8, const MA_Color *_Nonnull);
	void (*_Nonnull drawLineH)(struct ma_widget *_Nonnull, Uint8,Uint8, Uint8,
	                           const MA_Color *_Nonnull);
	void (*_Nonnull drawLineV)(struct ma_widget *_Nonnull, Uint8, Uint8,Uint8,
	                           const MA_Color *_Nonnull);
	void (*_Nonnull drawPolygon)(struct ma_widget *_Nonnull,
	                             const Uint8 *_Nonnull,
	                             Uint8, const MA_Color *_Nonnull);
	void (*_Nonnull drawBox)(struct ma_widget *_Nonnull,
	                         const MA_Rect *_Nonnull,
	                         Uint8,Uint8, const MA_Color *_Nonnull);
	void (*_Nonnull drawBoxRounded)(struct ma_widget *_Nonnull,
	                                const MA_Rect *_Nonnull, Sint8,
	                                Uint8, const MA_Color *_Nonnull);
	void (*_Nonnull drawBoxRoundedTop)(struct ma_widget *_Nonnull,
	                                   const MA_Rect *_Nonnull, Sint8,
					   Uint8, const MA_Color *_Nonnull);
	void (*_Nonnull drawCircle)(struct ma_widget *_Nonnull, Uint8,Uint8,
	                            Uint8, const MA_Color *_Nonnull);
	void (*_Nonnull drawCircleFilled)(struct ma_widget *_Nonnull, Uint8,Uint8,
	                                  Uint8, const MA_Color *_Nonnull);
	void (*_Nonnull drawRectFilled)(struct ma_widget *_Nonnull,
	                                const MA_Rect *_Nonnull,
	                                const MA_Color *_Nonnull);
} MA_DriverClass;

/* Generic driver instance. */
typedef struct ma_driver {
	struct ag_object _inherit;	/* AG_Object -> MA_Driver */
	Uint8 flags;
#define MA_DRIVER_USE_SOUND     0x01	/* Audio device is active */
#define MA_DRIVER_USE_MOUSE     0x02	/* Mouse is set up */
#define MA_DRIVER_USE_JOYSTICK  0x04	/* Joystick is set up */
#define MA_DRIVER_USE_CASSETTE  0x08	/* Cassette I/O is being done */
#define MA_DRIVER_USE_TIMER_A   0x10	/* Hardware timer A is in use */
#define MA_DRIVER_USE_TIMER_B   0x20	/* Hardware timer B is in use */
	Uint8 nSprites;			/* Number of hardware sprites in use */
} MA_Driver;

#define AGDRIVER(obj)            ((MA_Driver *)(obj))
#define AGCDRIVER(obj)           ((const MA_Driver *)(obj))
#define MA_DRIVER_SELF()          AGDRIVER( AG_OBJECT(0,"MA_Driver:*") )
#define MA_DRIVER_PTR(n)          AGDRIVER( AG_OBJECT((n),"MA_Driver:*") )
#define MA_DRIVER_NAMED(n)        AGDRIVER( AG_OBJECT_NAMED((n),"MA_Driver:*") )
#define MA_CONST_DRIVER_SELF()   AGCDRIVER( AG_CONST_OBJECT(0,"MA_Driver:*") )
#define MA_CONST_DRIVER_PTR(n)   AGCDRIVER( AG_CONST_OBJECT((n),"MA_Driver:*") )
#define MA_CONST_DRIVER_NAMED(n) AGCDRIVER( AG_CONST_OBJECT_NAMED((n),"MA_Driver:*") )

#define MADRIVER_CLASS(obj)	((struct ma_driver_class *)(AGOBJECT(obj)->cls))

#define MADRIVER_BOUNDED_WIDTH(win,x)  (((x) < 0) ? 0 : ((x) > MAWIDGET(win)->w) ? (MAWIDGET(win)->w - 1) : (x))
#define MADRIVER_BOUNDED_HEIGHT(win,y) (((y) < 0) ? 0 : ((y) > MAWIDGET(win)->h) ? (MAWIDGET(win)->h - 1) : (y))

__BEGIN_DECLS
extern AG_ObjectClass            maDriverClass;	/* Base MA_Driver class */
extern MA_Driver *_Nullable      maDriver;	/* Active driver instance */
extern MA_DriverClass *_Nullable maDriverOps;	/* Active driver ops */
extern MA_DriverClass *_Nonnull  maDriverList[];/* Available driver classes */

MA_Driver *_Nullable MA_DriverOpen(MA_DriverClass *_Nonnull);
void                 MA_DriverClose(MA_Driver *_Nonnull);

void MA_BeginRendering(void *_Nonnull);
void MA_EndRendering(void *_Nonnull);
void MA_GetDisplaySize(int *_Nonnull, int *_Nonnull);
__END_DECLS

#include <agar/micro/close.h>
#endif /* _AGAR_MICRO_DRV_H_ */
