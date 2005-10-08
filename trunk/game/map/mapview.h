/*	$Csoft: mapview.h,v 1.24 2005/09/20 13:46:31 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MAPEDIT_MAPVIEW_H_
#define _AGAR_MAPEDIT_MAPVIEW_H_

#include <engine/map/map.h>
#include <engine/map/mapedit.h>
#include <engine/map/tool.h>

#include <engine/actor.h>

#include <engine/widget/widget.h>

#include "begin_code.h"

struct ag_mapview;
struct ag_toolbar;
struct ag_label;
struct ag_tlist;
struct ag_scrollbar;

typedef struct ag_mapview_draw_cb {
	void (*func)(struct ag_mapview *, void *);
	void *p;
	SLIST_ENTRY(ag_mapview_draw_cb) draw_cbs;
} AG_MapviewDrawCb;

typedef struct ag_mapview {
	AG_Widget wid;

	int flags;
#define AG_MAPVIEW_EDIT		0x001	/* Mouse/keyboard edition */
#define AG_MAPVIEW_GRID		0x002	/* Display the grid */
#define AG_MAPVIEW_CENTER	0x004	/* Request initial centering */
#define AG_MAPVIEW_NO_CURSOR	0x008	/* Disable the cursor */
#define AG_MAPVIEW_NO_BMPSCALE	0x010	/* Disable bitmap scaling */
#define AG_MAPVIEW_NO_BG	0x020	/* Disable background tiles */ 
#define AG_MAPVIEW_NO_NODESEL	0x040	/* Disable node selections */
#define AG_MAPVIEW_SET_ATTRS	0x080	/* Setting node attributes */
#define AG_MAPVIEW_SHOW_OFFSETS	0x100	/* Show element tile offsets */
#define AG_MAPVIEW_SHOW_ORIGIN	0x200	/* Show map origin node */

	enum ag_mapview_mode {
		AG_MAPVIEW_EDITION,	/* Default edition mode */
		AG_MAPVIEW_EDIT_ATTRS,	/* Editing node attributes */
		AG_MAPVIEW_EDIT_ORIGIN,	/* Moving origin node */
		AG_MAPVIEW_PLAY		/* Playing mode */
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
		AG_Map map;		/* Temporary copy of the nodes */
		int x, y;		/* Origin of the rectangle */
		int w, h;		/* Dimensions of the rectangle */
	} esel;
	struct {			/* Noderef selection */
		int moving;		/* Noderefs are being displaced */
	} rsel;

	AG_Map *map;		/* Map to display */
	AG_Actor *actor;		/* Actor being controlled */
	int cam;			/* Name of map camera to use */
	int mx, my;			/* Display offset (nodes) */
	int xoffs, yoffs;		/* Display offset (pixels) */
	u_int mw, mh;			/* Display size (nodes) */

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

	AG_Maptool *curtool;		/* Selected tool */
	AG_Maptool *deftool;		/* Default tool if any */

	TAILQ_HEAD(, ag_maptool) tools;		    /* Map edition tools */
	SLIST_HEAD(, ag_mapview_draw_cb) draw_cbs;  /* Post-draw callbacks */
} AG_Mapview;

#define AGMCAM(mv)	(mv)->map->cameras[(mv)->cam]
#define AGMZOOM(mv)	AGMCAM(mv).zoom
#define AGMTILESZ(mv)	AGMCAM(mv).tilesz
#define AGMPIXSZ(mv)	AGMCAM(mv).pixsz

__BEGIN_DECLS
AG_Mapview	*AG_MapviewNew(void *, AG_Map *, int, struct ag_toolbar *,
		               struct ag_statusbar *);
void	 	 AG_MapviewInit(AG_Mapview *, AG_Map *, int,
		              struct ag_toolbar *, struct ag_statusbar *);

__inline__ void AG_MapviewPixel2i(AG_Mapview *, int, int);
__inline__ void AG_MapviewHLine(AG_Mapview *, int, int, int);
__inline__ void AG_MapviewVLine(AG_Mapview *, int, int, int);

void	 AG_MapviewDestroy(void *);
void	 AG_MapviewDraw(void *);
void	 AG_MapviewScale(void *, int, int);
void	 AG_MapviewPrescale(AG_Mapview *, int, int);
void	 AG_MapviewCenter(AG_Mapview *, int, int);
void	 AG_MapviewSetScale(AG_Mapview *, u_int, int);
void	 AG_MapviewSetSelection(AG_Mapview *, int, int, int, int);
int	 AG_MapviewGetSelection(AG_Mapview *, int *, int *, int *, int *);
void	 AG_MapviewRegDrawCb(AG_Mapview *, void (*)(AG_Mapview *, void *),
	                     void *);
void	 AG_MapviewUpdateCamera(AG_Mapview *);
void	 AG_MapviewUseScrollbars(AG_Mapview *, struct ag_scrollbar *,
		                 struct ag_scrollbar *);
void	 AG_MapviewStatus(AG_Mapview *, const char *, ...);
void	 AG_MapviewSetMode(AG_Mapview *, enum ag_mapview_mode);
void	 AG_MapviewControl(AG_Mapview *, const char *, void *);

#ifdef EDITION
AG_Maptool *AG_MapviewRegTool(AG_Mapview *, const AG_MaptoolOps *, void *);
AG_Maptool *AG_MapviewFindTool(AG_Mapview *, const char *);
void AG_MapviewSetDefaultTool(AG_Mapview *, AG_Maptool *);
void AG_MapviewSelectTool(AG_Mapview *, AG_Maptool *, void *);
#endif
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_MAPEDIT_MAPVIEW_H_ */
