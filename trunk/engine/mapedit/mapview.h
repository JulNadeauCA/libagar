/*	$Csoft: mapview.h,v 1.24 2003/02/13 11:30:12 vedge Exp $	*/
/*	Public domain	*/

struct mapview {
	struct widget	wid;

	int	flags;
#define MAPVIEW_EDIT		 0x001	/* Mouse/keyboard edition */
#define MAPVIEW_ZOOM		 0x002	/* Allow zooming */
#define MAPVIEW_INDEPENDENT_ZOOM 0x004	/* Zoom independent from map's */
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
	int		 mx, my;	/* Display offset (nodes) */
	int		 mw, mh;	/* Display size (nodes) */
	Sint16		 ssx, ssy;	/* Soft scrolling offsets */
	Uint16		*zoom;		/* Zoom (%) */
	SDL_TimerID	 zoom_tm;
	int		*tilew, *tileh;
	int		 cx, cy;	/* Cursor position (nodes) */
	struct {		/* For MAPVIEW_INDEPENDENT_ZOOM */
		Uint16	zoom;
		int	tilew, tileh;
	} izoom;
	struct {		/* For node edition */
		struct window	*win;		/* Node edition window */
		struct button	*button;
		struct label	*node_flags_lab, *node_size_lab;
		struct label	*noderef_type_lab, *noderef_flags_lab;
		struct label	*noderef_center_lab;
		struct tlist	*refs_tl;
		struct tlist	*transforms_tl;
	} node;
	struct window	*tmap_win;	/* Tile map window */
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
			    int, int);
