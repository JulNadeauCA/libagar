/*	$Csoft: mapview.h,v 1.19 2003/01/27 08:00:00 vedge Exp $	*/
/*	Public domain	*/

struct mapview {
	struct widget	wid;

	int	flags;
#define MAPVIEW_EDIT		 0x01	/* Mouse/keyboard edition */
#define MAPVIEW_ZOOM		 0x02	/* Allow zooming */
#define MAPVIEW_INDEPENDENT_ZOOM 0x04	/* Zoom independent from map's */
#define MAPVIEW_TILEMAP		 0x08	/* Map of `source' nodes */
#define MAPVIEW_GRID		 0x10	/* Display a grid */
#define MAPVIEW_PROPS		 0x20	/* Display node properties */

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
	int		mx, my;		/* Display offset (nodes) */
	int		mw, mh;		/* Display size (nodes) */
	Sint16		ssx, ssy;	/* Soft scrolling offsets */
	Uint16		*zoom;		/* Zoom (%) */
	int		*tilew, *tileh;
	int		cx, cy;		/* Cursor position (nodes) */

	struct {		/* For MAPVIEW_INDEPENDENT_ZOOM */
		Uint16	zoom;
		int	tilew, tileh;
	} izoom;

	struct window	*tmap_win;	/* Tile map window */
	struct button	*tmap_button;

	struct {		/* For node edition */
		struct window	*win;		/* Node edition window */
		struct button	*button;
		struct label	*node_flags_lab, *node_size_lab;
		struct label	*noderef_type_lab, *noderef_flags_lab;
		struct label	*noderef_center_lab;
		struct tlist	*refs_tl;
		struct tlist	*transforms_tl;
	} node;
};

enum mapview_prop_labels {
	MAPVIEW_FRAME_0,
	MAPVIEW_FRAME_1,
	MAPVIEW_FRAME_2,
	MAPVIEW_FRAME_3,
	MAPVIEW_FRAME_4,
	MAPVIEW_FRAME_5,
	MAPVIEW_FRAME_6,
	MAPVIEW_BLOCK,
	MAPVIEW_ORIGIN,
	MAPVIEW_WALK,
	MAPVIEW_CLIMB,
	MAPVIEW_SLIPPERY,
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

