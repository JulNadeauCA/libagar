/*	$Csoft$	*/
/*	Public domain	*/

struct eraser {
	struct	 tool tool;
	int	 flags;
	enum {
		ERASER_ALL = 0,		/* Erase all refs */
		ERASER_HIGHEST = 1,	/* Erase last ref */
		ERASER_LOWEST = 2,	/* Erase first ref */
		ERASER_SELECTIVE = 3	/* Erase specified refs */
	} mode;
	struct {
		struct	object *pobj;
		int	offs;
	} selection;
};

struct eraser	*eraser_new(struct mapedit *, int);
void		 eraser_init(struct eraser *, struct mapedit *, int);
struct window	*eraser_window(void *);
void		 eraser_effect(void *, struct map *, Uint32, Uint32);

