/*	$Csoft: eraser.h,v 1.3 2002/07/30 22:19:52 vedge Exp $	*/
/*	Public domain	*/

struct eraser {
	struct tool	tool;
	enum {
		ERASER_ALL,		/* Erase all refs */
		ERASER_HIGHEST,		/* Erase last ref */
		ERASER_LOWEST,		/* Erase first ref */
		ERASER_SELECTIVE	/* Erase specified refs */
	} mode;
	struct {
		struct object	*pobj;
		int		 offs;
	} selection;
};

struct eraser	*eraser_new(void);
void		 eraser_init(struct eraser *);
struct window	*eraser_window(void *);
void		 eraser_effect(void *, struct mapview *, Uint32, Uint32);

