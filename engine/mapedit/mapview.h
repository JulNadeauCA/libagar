/*	$Csoft: mapview.h,v 1.61 2004/11/30 11:49:45 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MAPEDIT_MAPVIEW_H_
#define _AGAR_MAPEDIT_MAPVIEW_H_

#include <engine/map.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/button.h>
#include <engine/widget/toolbar.h>
#include <engine/widget/statusbar.h>
#include <engine/widget/label.h>

#include <engine/mapedit/mapedit.h>
#include <engine/mapedit/tool.h>

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

	int prop_bg;			/* Background attributes style */
	int prop_style;			/* Foreground attributes style */
	int prew, preh;			/* Prescaling (nodes) */

	struct {			/* Mouse scrolling state */
		int scrolling;
		int centering;
		int x, y;
	} mouse;
	struct {			/* Temporary mouse selection */
		int set;
		int x, y;
		int xoffs, yoffs;
	} msel;
	struct {			/* Effective map selection */
		int set;
		int x, y;
		int w, h;
	} esel;

	u_int	zoom;			/* Zoom factor (%) */
	int	ssx, ssy;		/* Display offset */
	int	tilesz;			/* Current tile size */
	
	struct map	*map;		/* Map to display/edit */
	int	 	 mx, my;	/* Display offset (in nodes) */
	int		 xoffs, yoffs;	/* Absolute display coordinates */
	u_int	 	 mw, mh;	/* Display size (nodes) */
	int		 wfit, hfit;	/* Dimension fits into display? */
	int		 wmod, hmod;	/* Remainders */
	int		 cx, cy;	/* Cursor position (nodes) */
	int		 cxrel, cyrel;	/* Displacement from last position */
	int		 dblclicked;

	struct toolbar *toolbar;	/* Optional toolbar */
	struct statusbar *statusbar;	/* Optional status bar */
	struct label *status;		/* Optional status label */
	struct tool *curtool;
	TAILQ_HEAD(, tool) tools;
	SLIST_HEAD(, mapview_draw_cb) draw_cbs;
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
void	 mapview_set_scale(struct mapview *, u_int);
void	 mapview_set_selection(struct mapview *, int, int, int, int);
int	 mapview_get_selection(struct mapview *, int *, int *, int *, int *);

__inline__ void	 mapview_ncoords(struct mapview *, int *, int *, int *, int *);
void	 	 mapview_reg_draw_cb(struct mapview *,
				     void (*)(struct mapview *, void *),
				     void *);

#ifdef EDITION
struct tool	*mapview_reg_tool(struct mapview *, const struct tool *,
		                  void *, int);
void	 	 mapview_select_tool(struct mapview *, struct tool *, void *);
void		 mapview_selected_layer(int, union evarg *);
#endif

void	 mapview_status(struct mapview *, const char *, ...);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_MAPEDIT_MAPVIEW_H_ */
