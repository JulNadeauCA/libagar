/*	$Csoft: eraser.h,v 1.8 2003/02/12 01:09:47 vedge Exp $	*/
/*	Public domain	*/

enum eraser_mode {
	ERASER_ALL,
	ERASER_HIGHEST
};

struct eraser {
	struct tool	tool;
	int		mode;			/* Eraser mode */
	int		all_layers;		/* Apply to every layer */
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
