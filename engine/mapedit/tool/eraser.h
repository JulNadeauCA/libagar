/*	$Csoft: eraser.h,v 1.2 2002/07/09 08:25:21 vedge Exp $	*/
/*	Public domain	*/

struct eraser {
	struct	 tool tool;
	int	 flags;
	enum {
		ERASER_ALL,		/* Erase all refs */
		ERASER_HIGHEST,		/* Erase last ref */
		ERASER_LOWEST,		/* Erase first ref */
		ERASER_SELECTIVE	/* Erase specified refs */
	} mode;
	struct {
		struct	object *pobj;
		int	offs;
	} selection;
};

struct eraser	*eraser_new(struct mapedit *, int);
void		 eraser_init(struct eraser *, struct mapedit *, int);
struct window	*eraser_window(void *);
void		 eraser_effect(void *, struct mapview *, Uint32, Uint32);

