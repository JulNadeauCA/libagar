/*	$Csoft: mapview.h,v 1.29 2003/03/02 07:29:53 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/nodeedit.h>
#include <engine/mapedit/layedit.h>

struct mapview {
	struct widget	wid;

	int	flags;
#define MAPVIEW_EDIT		 0x001	/* Mouse/keyboard edition */
#define MAPVIEW_ZOOM		 0x002	/* Allow zooming */
#define MAPVIEW_INDEPENDENT	 0x004	/* Zoom/ss[xy] independent from map's */
#define MAPVIEW_TILEMAP		 0x008	/* Map of `source' nodes */
#define MAPVIEW_GRID		 0x010	/* Display a grid */
#define MAPVIEW_PROPS		 0x020	/* Display node properties */
#define MAPVIEW_ZOOMING_IN	 0x040
#define MAPVIEW_ZOOMING_OUT	 0x080
#define MAPVIEW_CENTER		 0x100
#define MAPVIEW_SAVEABLE	 0x200	/* Load/save keys */
#define MAPVIEW_NO_CURSOR	 0x400	/* Hide cursor */
	int	 prop_bg;	/* Background of node attributes */
	int	 prop_style;	/* Style of node attributes */
	struct {
		int	scrolling;	/* Currently scrolling? */
		int	x, y;		/* Current mouse position */
	} mouse;
	struct {		/* For inserting tiles dynamically */
		enum {
			MAPVIEW_CONSTR_HORIZ,	/* Horizontally, grow */
			MAPVIEW_CONSTR_VERT,	/* Vertically, grow */
			MAPVIEW_CONSTR_FILL	/* Fill available nodes */
		} mode;
		int	x, y;		/* Current position */
		int	nflags;		/* Noderef flags */
	} constr;
	struct map	*map;
	struct node	*cur_node;
	Uint8		 cur_layer;
	int		 mx, my;	/* Display offset (nodes) */
	int		 mw, mh;	/* Display size (nodes) */
	Uint16		*zoom;		/* Zoom (%) */
	Sint16		*ssx, *ssy;	/* Soft scroll offsets */
	SDL_TimerID	 zoom_tm;
	int		*tilew, *tileh;
	int		 cx, cy;	/* Cursor position (nodes) */
	int		 cw, ch;	/* Cursor geometry (nodes) */
	struct {		/* For MAPVIEW_INDEPENDENT */
		Uint16	zoom;
		Sint16	ssx, ssy;
		int	tilew, tileh;
	} izoom;
	struct nodeedit	 nodeed;
	struct layedit	 layed;
	struct window	*tmap_win;	/* Tile map window */
	int		 tmap_insert;	/* Insert mode? */
	struct button	*tmap_button;
};

enum mapview_prop_labels {
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

struct mapview	*mapview_new(struct region *, struct map *, int, int, int);
void		 mapview_init(struct mapview *, struct map *, int, int, int);
void		 mapview_destroy(void *);

void		 mapview_node_edit_win(struct mapview *);
void		 mapview_draw(void *);
void		 mapview_center(struct mapview *, int, int);
void		 mapview_zoom(struct mapview *, int);

extern __inline__ void	mapview_draw_props(struct mapview *, struct node *,
			    int, int, int, int);
