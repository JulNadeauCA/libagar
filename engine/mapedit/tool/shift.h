/*	$Csoft: shift.h,v 1.3 2003/02/02 21:14:02 vedge Exp $	*/
/*	Public domain	*/

enum shift_mode {
	SHIFT_HIGHEST,
	SHIFT_LOWEST,
	SHIFT_SELECTIVE,
	SHIFT_ALL
};

struct shift {
	struct tool	tool;
	int		mode;
	int		all_layers;
};

void		 shift_init(void *);
struct window	*shift_window(void *);
void		 shift_effect(void *, struct mapview *, struct node *);
void		 shift_mouse(void *, struct mapview *, Sint16, Sint16);

