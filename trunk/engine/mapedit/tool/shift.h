/*	$Csoft: shift.h,v 1.3 2003/01/19 12:09:42 vedge Exp $	*/
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

struct shift	*shift_new(void);
void		 shift_init(struct shift *);
struct window	*shift_window(void *);
void		 shift_effect(void *, struct mapview *, Uint32, Uint32);
void		 shift_mouse(void *, struct mapview *, Sint16, Sint16);

