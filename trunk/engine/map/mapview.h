/*	$Csoft: mapview.h,v 1.8 2005/06/16 02:54:40 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MAPEDIT_MAPVIEW_H_
#define _AGAR_MAPEDIT_MAPVIEW_H_

#include <engine/map/map.h>
#include <engine/map/mapedit.h>
#include <engine/map/tool.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/button.h>
#include <engine/widget/toolbar.h>
#include <engine/widget/statusbar.h>
#include <engine/widget/scrollbar.h>
#include <engine/widget/label.h>

#include "begin_code.h"

struct mapview_draw_cb {
	void (*func)(struct mapview *, void *);
	void *p;
	SLIST_ENTRY(mapview_draw_cb) draw_cbs;
};

struct mapview {
	struct widget wid;

	int flags;
#define MAPVIEW_EDIT		0x01	/* Mouse/keyboard edition */
#define MAPVIEW_GRID		0x02	/* Display the grid */
#define MAPVIEW_PROPS		0x04	/* Display node properties */
#define MAPVIEW_CENTER		0x08	/* Request initial centering */
#define MAPVIEW_NO_CURSOR	0x10	/* Disable the cursor */
#define MAPVIEW_NO_BMPZOOM	0x20	/* Disable bitmap scaling */
#define MAPVIEW_NO_BG		0x40	/* Disable background tiles */ 

	int prop_bg;			/* Background attributes style */
	int prop_style;			/* Foreground attributes style */
	int prew, preh;			/* Prescaling (nodes) */

	struct {			/* Mouse scrolling state */
		int scrolling;
		int x, y;
	} mouse;
	struct {			/* Temporary mouse selection */
		int set;		/* Selection is set */
		int x, y;
		int xoffs, yoffs;
	} msel;
	struct {			/* Effective map selection */
		int set;		/* Selection is set */
		int moving;		/* Nodes are being displaced */
		struct map map;		/* Temporary copy of the nodes */
		int x, y;
		int w, h;
	} esel;

	u_int zoom;			/* Zoom factor (%) */
	int tilesz;			/* Display tile size (pixels) */
	int pxsz;			/* Scaled pixel size (pixels) */
	
	struct map *map;		/* Map to display */
	int cam;			/* Name of map camera to use */

	int mx, my;			/* Display offset (nodes) */
	int xoffs, yoffs;		/* Display offset (pixels) */
	u_int mw, mh;			/* Display size (nodes) */

	int cx, cy;			/* Cursor position (nodes) */
	int cxoffs, cyoffs;		/* Cursor offset (pixels) */
	int cxrel, cyrel;		/* Displacement from last position */
	int cxabs, cyabs;		/* Absolute cursor position (pixels) */
	int dblclicked;			/* Double click flag */

	struct toolbar *toolbar;	/* Optional toolbar */
	struct statusbar *statusbar;	/* Optional status bar */
	struct label *status;		/* Optional status label */
	struct tlist *art_tl;		/* Optional artwork list */
	struct tlist *objs_tl;		/* Optional object list */
	struct tlist *layers_tl;	/* Optional layer list */

	struct scrollbar *vbar, *hbar;	/* Scrollbars (or NULL) */

	struct tool *curtool;			/* Selected tool */
	TAILQ_HEAD(, tool) tools;		/* Map edition tools */
	SLIST_HEAD(, mapview_draw_cb) draw_cbs;	/* Post-draw callbacks */
};

enum mapview_prop_labels {
	MAPVIEW_BASE,
	MAPVIEW_FRAME_0,
	MAPVIEW_FRAME_1,
	MAPVIEW_FRAME_2,
	MAPVIEW_FRAME_3,
	MAPVIEW_FRAME_4,
	MAPVIEW_FRAME_5,
	MAPVIEW_FRAME_6,
	MAPVIEW_FRAMES_END,
	MAPVIEW_BLOCK,
	MAPVIEW_ORIGIN,
	MAPVIEW_WALK,
	MAPVIEW_CLIMB,
	MAPVIEW_SLIPPERY,
	MAPVIEW_EDGE,
	MAPVIEW_EDGE_N,
	MAPVIEW_EDGE_S,
	MAPVIEW_EDGE_W,
	MAPVIEW_EDGE_E,
	MAPVIEW_EDGE_NW,
	MAPVIEW_EDGE_NE,
	MAPVIEW_EDGE_SW,
	MAPVIEW_EDGE_SE,
	MAPVIEW_BIO,
	MAPVIEW_REGEN,
	MAPVIEW_SLOW,
	MAPVIEW_HASTE
};

struct node;

#define MAPVIEW_CAM(mv) (mv)->map->cameras[(mv)->cam]

__BEGIN_DECLS
struct mapview	*mapview_new(void *, struct map *, int, struct toolbar *,
		             struct statusbar *);
void	 	 mapview_init(struct mapview *, struct map *, int,
		              struct toolbar *, struct statusbar *);

void	 mapview_destroy(void *);
void	 mapview_draw(void *);
void	 mapview_scale(void *, int, int);
void	 mapview_prescale(struct mapview *, int, int);
void	 mapview_draw_props(struct mapview *, struct node *, int, int, int,
	                    int);
void	 mapview_center(struct mapview *, int, int);
void	 mapview_set_scale(struct mapview *, u_int, int);
void	 mapview_set_selection(struct mapview *, int, int, int, int);
int	 mapview_get_selection(struct mapview *, int *, int *, int *, int *);
void	 mapview_reg_draw_cb(struct mapview *,
	                     void (*)(struct mapview *, void *), void *);
void	 mapview_update_camera(struct mapview *);
void	 mapview_set_scrollbars(struct mapview *, struct scrollbar *,
		                struct scrollbar *);
void	 mapview_status(struct mapview *, const char *, ...);

#ifdef EDITION
struct tool *mapview_reg_tool(struct mapview *, const struct tool *, void *);
void mapview_select_tool(struct mapview *, struct tool *, void *);
void mapview_selected_layer(int, union evarg *);
#endif
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_MAPEDIT_MAPVIEW_H_ */
