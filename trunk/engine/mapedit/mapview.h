/*	$Csoft: mapview.h,v 1.40 2003/04/25 09:47:07 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MAPEDIT_MAPVIEW_H_
#define _AGAR_MAPEDIT_MAPVIEW_H_

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/button.h>

#include <engine/mapedit/nodeedit.h>
#include <engine/mapedit/layedit.h>

#include "begin_code.h"

/* For construction of source tile maps. */
struct mapview_constr {
	int		 x, y;		/* Current position */
	struct window	*win;		/* Source tiles window */
	int		 replace;	/* Replace mode? */
	struct button	*trigger;
};

struct mapview {
	struct widget	wid;

	int	flags;
#define MAPVIEW_EDIT		 0x001	/* Mouse/keyboard edition */
#define MAPVIEW_INDEPENDENT	 0x002	/* Zoom/ss independent from map's */
#define MAPVIEW_TILEMAP		 0x004	/* Map of `source' nodes */
#define MAPVIEW_GRID		 0x008	/* Display a grid */
#define MAPVIEW_PROPS		 0x010	/* Display node properties */
#define MAPVIEW_ZOOMING_IN	 0x020
#define MAPVIEW_ZOOMING_OUT	 0x040
#define MAPVIEW_CENTER		 0x080
#define MAPVIEW_SAVEABLE	 0x100	/* Load/save keys */
#define MAPVIEW_NO_CURSOR	 0x200	/* Hide cursor */
	int	 prop_bg;		/* Background of node attributes */
	int	 prop_style;		/* Style of node attributes */

	struct {		/* Mouse scrolling state */
		int	scrolling;
		int	centering;
		int	x, y;
	} mouse;
	struct {		/* Temporary mouse selection */
		int	set;
		int	x, y;		/* Selection origin */
		int	xoffs, yoffs;	/* Displacement from origin */
	} msel;
	struct {		/* Effective map selection */
		int	set;
		int	x, y;	/* Rectangle origin */
		int	w, h;	/* Rectangle geometry */
	} esel;

	Uint16		*zoom;		/* Zoom factor (%) */
	int		 zoom_inc;	/* Zoom increment (%) */
	int		 zoom_ival;	/* Zoom interval (ms) */
	SDL_TimerID	 zoom_tm;	/* Zoom timer */

	Sint16		*ssx, *ssy;	/* Soft scroll offsets */
	int		*tilew, *tileh;	/* Current tile geometry */

	struct {		/* For MAPVIEW_INDEPENDENT zoom */
		Uint16	 zoom;
		Sint16	 ssx, ssy;
		int	 tilew, tileh;
	} izoom;

	struct map	*map;		/* Map to display */
	int		 mx, my;	/* Display offset (nodes) */
	unsigned int	 mw, mh;	/* Display size (nodes) */
	int		 cx, cy;	/* Cursor position (nodes) */
	int		 cxrel, cyrel;	/* Displacement from last position */

	struct mapview_constr	constr;	/* Source tile mapping */
	struct nodeedit		nodeed;	/* Node editor */
	struct layedit		layed;	/* Layer editor */
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
extern DECLSPEC struct mapview	*mapview_new(struct region *, struct map *, int,
				             int, int);
extern DECLSPEC void		 mapview_init(struct mapview *, struct map *,
				              int, int, int);
extern DECLSPEC void		 mapview_destroy(void *);

extern DECLSPEC void	 mapview_node_edit_win(struct mapview *);
extern DECLSPEC void	 mapview_draw(void *);
extern DECLSPEC void	 mapview_draw_props(struct mapview *, struct node *,
			                    int, int, int, int);
extern DECLSPEC void	 mapview_center(struct mapview *, int, int);
extern DECLSPEC int	 mapview_zoom(struct mapview *, int);
extern DECLSPEC void	 mapview_map_coords(struct mapview *, int *, int *);
extern DECLSPEC void	 mapview_set_selection(struct mapview *, int, int, int,
			                       int);
extern DECLSPEC int	 mapview_get_selection(struct mapview *, int *, int *,
			                       int *, int *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_MAPEDIT_MAPVIEW_H_ */
