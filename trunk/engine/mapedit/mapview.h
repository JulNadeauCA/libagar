/*	$Csoft: mapview.h,v 1.2 2002/06/23 02:38:21 vedge Exp $	*/
/*	Public domain	*/

struct edcursor;
struct mapedit;

struct mapview {
	struct	 widget wid;

	int	flags;
#define MAPVIEW_CENTER	0x01	/* Center on cursor */

	struct {		/* For scrolling */
		int	move;
		Sint16	x;
		Sint16	y;
	} mouse;

	struct	edcursor *cursor;

	struct	map *map;
	int	mx, my;		/* Map offset */
	int	mw, mh;		/* Size in nodes */

	struct	mapedit *med;	/* Back pointer to map editor */
	struct	mapview *mv;	/* Back pointer to map view widget */
};

struct mapview	*mapview_new(struct region *, struct mapedit *, struct map *,
		     int, int, int);
void		 mapview_init(struct mapview *, struct mapedit *, struct map *,
		     int, int, int);
void		 mapview_destroy(void *);

void	 mapview_draw(void *);
void	 mapview_center(struct mapview *, int, int);

