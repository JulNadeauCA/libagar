/*	$Csoft: eraser.h,v 1.7 2003/02/02 21:14:02 vedge Exp $	*/
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
void		 eraser_effect(void *, struct mapview *, struct node *);
int		 eraser_load(void *, int);
int		 eraser_save(void *, int);
