/*	$Csoft: resize.h,v 1.4 2003/02/02 21:14:02 vedge Exp $	*/
/*	Public domain	*/

struct resize {
	struct tool	tool;
	int		cx, cy;
	enum {
		RESIZE_GROW,
		RESIZE_SHRINK
	} mode;
};

void		 resize_init(void *);
struct window	*resize_window(void *);
void		 resize_mouse(void *p, struct mapview *mv, Sint16, Sint16,
		     Uint8);

