/*	$Csoft: mapview.h,v 1.15 2003/01/19 12:09:40 vedge Exp $	*/
/*	Public domain	*/

struct mapedit;

struct mapview {
	struct widget	wid;

	int	flags;
#define MAPVIEW_CENTER		0x01	/* Center on cursor */
#define MAPVIEW_EDIT		0x02	/* Mouse/keyboard edition */
#define MAPVIEW_ZOOM		0x04	/* Allow zooming */
#define MAPVIEW_TILEMAP		0x08	/* Tile map */
#define MAPVIEW_GRID		0x10	/* Display a grid */
#define MAPVIEW_PROPS		0x20	/* Display node properties */

	int	 prop_bg;	/* Background of node attributes */
	int	 prop_style;	/* Style of node attributes */

	struct {		/* For scrolling */
		int	move;		/* Currently scrolling? */
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
	int		mx, my;		/* Display offset */
	int		mw, mh;		/* Display size in nodes */

	struct mapedit	*med;		/* Back pointer to map editor */
	struct window	*tmap_win;	/* Tile map window */
	struct button	*tmap_button;
	struct window	*node_win;	/* Node edition window */
	struct button	*node_button;
	struct tlist	*node_tlist;	/* Node edition list */
};

struct mapview	*mapview_new(struct region *, struct mapedit *, struct map *,
		     int, int, int);
void		 mapview_init(struct mapview *, struct mapedit *, struct map *,
		     int, int, int);
void		 mapview_destroy(void *);

void		 mapview_node_edit_win(struct mapview *);
void		 mapview_draw(void *);
void		 mapview_center(struct mapview *, int, int);
void		 mapview_zoom(struct mapview *, int);

