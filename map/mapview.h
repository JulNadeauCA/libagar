/*	Public domain	*/

#ifndef _AGAR_MAPEDIT_MAPVIEW_H_
#define _AGAR_MAPEDIT_MAPVIEW_H_

#ifdef _AGAR_INTERNAL
#include <map/map.h>
#include <map/mapedit.h>
#include <map/tool.h>
#include <map/actor.h>
#include <gui/widget.h>
#else
#include <agar/map/map.h>
#include <agar/map/mapedit.h>
#include <agar/map/tool.h>
#include <agar/map/actor.h>
#include <agar/gui/widget.h>
#endif

#include "begin_code.h"

struct map_view;
struct ag_toolbar;
struct ag_label;
struct ag_tlist;
struct ag_scrollbar;

typedef struct map_view_draw_cb {
	void (*func)(struct map_view *, void *);
	void *p;
	SLIST_ENTRY(map_view_draw_cb) draw_cbs;
} MAP_ViewDrawCb;

typedef struct map_view {
	AG_Widget wid;

	int flags;
#define MAP_VIEW_EDIT		0x001	/* Mouse/keyboard edition */
#define MAP_VIEW_GRID		0x002	/* Display the grid */
#define MAP_VIEW_CENTER	0x004	/* Request initial centering */
#define MAP_VIEW_NO_CURSOR	0x008	/* Disable the cursor */
#define MAP_VIEW_NO_BMPSCALE	0x010	/* Disable bitmap scaling */
#define MAP_VIEW_NO_BG	0x020	/* Disable background tiles */ 
#define MAP_VIEW_NO_NODESEL	0x040	/* Disable node selections */
#define MAP_VIEW_SET_ATTRS	0x080	/* Setting node attributes */
#define MAP_VIEW_SHOW_OFFSETS	0x100	/* Show element tile offsets */
#define MAP_VIEW_SHOW_ORIGIN	0x200	/* Show map origin node */

	enum map_view_mode {
		MAP_VIEW_EDITION,	/* Default edition mode */
		MAP_VIEW_EDIT_ATTRS,	/* Editing node attributes */
		MAP_VIEW_EDIT_ORIGIN,	/* Moving origin node */
		MAP_VIEW_PLAY		/* Playing mode */
	} mode;

	int edit_attr;			/* Attribute being edited */
	int attr_x, attr_y;

	int prew, preh;			/* Prescaling (nodes) */

	struct {
		Uint8 r, g, b, a;	/* Current color (for agPrim) */
		Uint32 pixval;
	} col;
	struct {			/* Mouse scrolling state */
		int scrolling;		/* Scrolling is in progress */
		int x, y;		/* Current node coordinates */
		int xmap, ymap;		/* Current map coordinates */
		int xmap_rel, ymap_rel;	/* Relative map coordinates */
	} mouse;
	struct {			/* Temporary mouse selection */
		int set;		/* Selection is set */
		int x, y;		/* Origin of rectangle */
		int xoffs, yoffs;	/* Displacement from origin */
	} msel;
	struct {			/* Effective map selection */
		int set;		/* Selection is set */
		int moving;		/* Nodes are being displaced */
		MAP map;		/* Temporary copy of the nodes */
		int x, y;		/* Origin of the rectangle */
		int w, h;		/* Dimensions of the rectangle */
	} esel;
	struct {			/* Noderef selection */
		int moving;		/* Noderefs are being displaced */
	} rsel;

	MAP *map;		/* Map to display */
	MAP_Actor *actor;		/* Actor being controlled */
	int cam;			/* Name of map camera to use */
	int mx, my;			/* Display offset (nodes) */
	int xoffs, yoffs;		/* Display offset (pixels) */
	Uint mw, mh;			/* Display size (nodes) */

	int cx, cy;			/* Cursor position (nodes) */
	int cxoffs, cyoffs;		/* Cursor offset (pixels) */
	int cxrel, cyrel;		/* Relative displacement (nodes) */
	int dblclicked;			/* Double click flag */

	struct ag_toolbar *toolbar;	/* Optional toolbar */
	struct ag_statusbar *statusbar;	/* Optional status bar */
	struct ag_label *status;	/* Optional status label */
	struct ag_tlist *lib_tl;	/* Optional library list */
	struct ag_tlist *objs_tl;	/* Optional library list */
	struct ag_tlist *layers_tl;	/* Optional layer list */
	struct ag_scrollbar *vbar, *hbar; /* Optional scrollbars */

	MAP_Tool *curtool;		/* Selected tool */
	MAP_Tool *deftool;		/* Default tool if any */

	TAILQ_HEAD(, map_tool) tools;		    /* Map edition tools */
	SLIST_HEAD(, map_view_draw_cb) draw_cbs;    /* Post-draw callbacks */
} MAP_View;

#define AGMCAM(mv)	(mv)->map->cameras[(mv)->cam]
#define AGMZOOM(mv)	AGMCAM(mv).zoom
#define AGMTILESZ(mv)	AGMCAM(mv).tilesz
#define AGMPIXSZ(mv)	AGMCAM(mv).pixsz

__BEGIN_DECLS
extern const AG_WidgetOps mapViewOps;

MAP_View	*MAP_ViewNew(void *, MAP *, int, struct ag_toolbar *,
		               struct ag_statusbar *);
void	 	 MAP_ViewInit(MAP_View *, MAP *, int,
		              struct ag_toolbar *, struct ag_statusbar *);

__inline__ void MAP_ViewPixel2i(MAP_View *, int, int);
__inline__ void MAP_ViewHLine(MAP_View *, int, int, int);
__inline__ void MAP_ViewVLine(MAP_View *, int, int, int);

void	 MAP_ViewPrescale(MAP_View *, int, int);
void	 MAP_ViewCenter(MAP_View *, int, int);
void	 MAP_ViewSetScale(MAP_View *, Uint, int);
void	 MAP_ViewSetSelection(MAP_View *, int, int, int, int);
int	 MAP_ViewGetSelection(MAP_View *, int *, int *, int *, int *);
void	 MAP_ViewRegDrawCb(MAP_View *, void (*)(MAP_View *, void *),
	                     void *);
void	 MAP_ViewUpdateCamera(MAP_View *);
void	 MAP_ViewUseScrollbars(MAP_View *, struct ag_scrollbar *,
		                 struct ag_scrollbar *);
void	 MAP_ViewStatus(MAP_View *, const char *, ...);
void	 MAP_ViewSetMode(MAP_View *, enum map_view_mode);
void	 MAP_ViewControl(MAP_View *, const char *, void *);

MAP_Tool *MAP_ViewRegTool(MAP_View *, const MAP_ToolOps *, void *);
MAP_Tool *MAP_ViewFindTool(MAP_View *, const char *);
void MAP_ViewSetDefaultTool(MAP_View *, MAP_Tool *);
void MAP_ViewSelectTool(MAP_View *, MAP_Tool *, void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_MAPEDIT_MAPVIEW_H_ */
