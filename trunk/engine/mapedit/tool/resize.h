/*	$Csoft: resize.h,v 1.2 2003/01/19 12:09:42 vedge Exp $	*/
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
void		 resize_effect(void *, struct mapview *, Uint32, Uint32);

