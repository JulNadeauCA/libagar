/*	$Csoft: mapview.h,v 1.51 2004/03/17 12:42:06 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MAPEDIT_MAPVIEW_H_
#define _AGAR_MAPEDIT_MAPVIEW_H_

#include <engine/map.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/button.h>
#include <engine/widget/toolbar.h>

#include <engine/mapedit/tool.h>
#include <engine/mapedit/nodeedit.h>
#include <engine/mapedit/layedit.h>
#include <engine/mapedit/mediasel.h>

#include "begin_code.h"

struct mapview {
	struct widget wid;

	int flags;
#define MAPVIEW_EDIT		0x0001	/* Mouse/keyboard edition */
#define MAPVIEW_INDEPENDENT	0x0002	/* Independent zoom/scroll */
#define MAPVIEW_GRID		0x0004	/* Display the grid */
#define MAPVIEW_PROPS		0x0008	/* Display node properties */
#define MAPVIEW_ZOOMING_IN	0x0010	/* Zoomin in progression */
#define MAPVIEW_ZOOMING_OUT	0x0020	/* Zoomout in progression */
#define MAPVIEW_CENTER		0x0040	/* Request initial centering */
#define MAPVIEW_NO_CURSOR	0x0080	/* Disable the cursor */

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

	Uint16		*zoom;		/* Zoom factor (%) */
	int		 zoom_inc;	/* Zoom increment (%) */
	int		 zoom_ival;	/* Zoom interval (ms) */
	SDL_TimerID	 zoom_tm;	/* Zoom timer */
	Sint16		*ssx, *ssy;	/* Soft scroll offsets */
	int		*scale;		/* Current scale factor */
	struct {			/* For MAPVIEW_INDEPENDENT zoom */
		Uint16	 zoom;
		Sint16	 ssx, ssy;
		int	 scale;
	} izoom;

	struct map	*map;		/* Map to display/edit */
	int		 mx, my;	/* Display offset (nodes) */
	unsigned int	 mw, mh;	/* Display size (nodes) */
	int		 cx, cy;	/* Cursor position (nodes) */
	int		 cxrel, cyrel;	/* Displacement from last position */

	struct nodeedit	nodeed;		/* Node editor */
	struct layedit	layed;		/* Layer editor */
	struct {
		struct window *win;
		struct button *trigger;
		struct mediasel *gfx;
		struct mediasel *audio;
	} mediasel;

	struct toolbar *toolbar;	/* Pointer to toolbar (or NULL) */
	struct tool *curtool;		/* Selected tool */
	TAILQ_HEAD(, tool) tools;	/* Map edition tools */
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
struct mapview	*mapview_new(void *, struct map *, int, struct toolbar *);

void	 mapview_init(struct mapview *, struct map *, int, struct toolbar *);
void	 mapview_destroy(void *);
void	 mapview_draw(void *);
void	 mapview_scale(void *, int, int);
void	 mapview_prescale(struct mapview *, int, int);
void	 mapview_draw_props(struct mapview *, struct node *, int, int, int,
	                    int);
void	 mapview_center(struct mapview *, int, int);
int	 mapview_zoom(struct mapview *, int);
void	 mapview_set_selection(struct mapview *, int, int, int, int);
int	 mapview_get_selection(struct mapview *, int *, int *, int *, int *);

__inline__ void	 mapview_map_coords(struct mapview *, int *, int *);

#ifdef EDITION
void	 mapview_reg_tool(struct mapview *, const struct tool *);
void	 mapview_select_tool(int, union evarg *);
void	 mapview_toggle_rw(int, union evarg *);
void	 mapview_toggle_nodeedit(int, union evarg *);
void	 mapview_toggle_layedit(int, union evarg *);
void	 mapview_toggle_mediasel(int, union evarg *);
void	 mapview_selected_layer(int, union evarg *);
#endif
void	 mapview_toggle_grid(int, union evarg *);
void	 mapview_toggle_props(int, union evarg *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_MAPEDIT_MAPVIEW_H_ */
