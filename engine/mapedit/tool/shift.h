/*	$Csoft: shift.h,v 1.4 2003/03/07 03:24:49 vedge Exp $	*/
/*	Public domain	*/

enum shift_mode {
	SHIFT_HIGHEST,
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

