/*	$Csoft: mapview.h,v 1.10 2002/08/19 05:33:02 vedge Exp $	*/
/*	Public domain	*/

struct mapedit;

struct mapview {
	struct	 widget wid;

	int	flags;
#define MAPVIEW_CENTER		0x01	/* Center on cursor */
#define MAPVIEW_EDIT		0x02	/* Mouse/keyboard edition */
#define MAPVIEW_ZOOM		0x04	/* Allow zooming */
#define MAPVIEW_TILEMAP		0x08	/* Tile map */
#define MAPVIEW_GRID		0x10	/* Display a grid */
#define MAPVIEW_PROPS		0x20	/* Display node properties */
#define MAPVIEW_SHOW_CURSOR	0x40	/* Show mouse cursor */

	int	 prop_style;	/* Attribute background (or -1) */

	struct {		/* For scrolling */
		int	move;
		Sint16	x;
		Sint16	y;
	} mouse;

	struct	map *map;
	int	mx, my;		/* Map offset */
	int	mw, mh;		/* Size in nodes */
	int	tilew, tileh;	/* Tile geometry */
	int	zoom;		/* Zoom (%) */

	struct	mapedit *med;	/* Back pointer to map editor */
};

struct mapview	*mapview_new(struct region *, struct mapedit *, struct map *,
		     int, int, int);
void		 mapview_init(struct mapview *, struct mapedit *, struct map *,
		     int, int, int);
void		 mapview_destroy(void *);

void	 mapview_draw(void *);
void	 mapview_center(struct mapview *, int, int);
void	 mapview_zoom(struct mapview *, int);

