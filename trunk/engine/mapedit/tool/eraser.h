/*	$Csoft: eraser.h,v 1.5 2003/01/25 06:29:30 vedge Exp $	*/
/*	Public domain	*/

enum eraser_mode {
	ERASER_ALL,		/* Erase all refs */
	ERASER_HIGHEST,		/* Erase last ref */
	ERASER_LOWEST,		/* Erase first ref */
	ERASER_SELECTIVE	/* Erase specified refs */
};

struct eraser {
	struct tool	tool;

	int	mode;				/* Eraser mode */
	struct {
		struct object	*pobj;
		int		 offs;		/* -1 = any */
	} selection;
};

void		 eraser_init(void *);
struct window	*eraser_window(void *);
void		 eraser_effect(void *, struct mapview *, Uint32, Uint32);

