/*	$Csoft: shift.h,v 1.2 2003/01/26 06:15:21 vedge Exp $	*/
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
};

void		 shift_init(void *);
struct window	*shift_window(void *);
void		 shift_effect(void *, struct mapview *, struct node *);
void		 shift_mouse(void *, struct mapview *, Sint16, Sint16);

