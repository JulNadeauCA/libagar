/*	$Csoft: resize.h,v 1.5 2003/02/22 11:44:05 vedge Exp $	*/
/*	Public domain	*/

struct resize {
	struct tool	tool;
	enum {
		RESIZE_GROW,
		RESIZE_SHRINK
	} mode;
};

void		 resize_init(void *);
struct window	*resize_window(void *);
void		 resize_mouse(void *, struct mapview *, Sint16, Sint16,
		     Uint8);

