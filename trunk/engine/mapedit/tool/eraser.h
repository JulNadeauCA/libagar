/*	$Csoft: eraser.h,v 1.9 2003/03/07 03:24:49 vedge Exp $	*/
/*	Public domain	*/

enum eraser_mode {
	ERASER_ALL,
	ERASER_HIGHEST
};

struct eraser {
	struct tool	tool;
	int		mode;			/* Eraser mode */
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
