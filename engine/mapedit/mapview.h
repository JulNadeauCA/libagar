/*	$Csoft$	*/
/*	Public domain	*/

struct mapview {
	struct	 widget wid;

	int	flags;

	struct {
		int	move;
		Sint16	x;
		Sint16	y;
	} mouse;

	struct	map *map;
	int	mx, my;		/* Map offset */
	int	mw, mh;		/* Size in nodes */
};

struct mapview	*mapview_new(struct region *, struct map *, int, int, int);
void		 mapview_init(struct mapview *, struct map *, int, int, int);
void		 mapview_destroy(void *);

void	 mapview_draw(void *);
void	 mapview_animate(void *);

