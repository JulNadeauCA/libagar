/*	$Csoft: resize.h,v 1.3 2003/01/26 06:15:21 vedge Exp $	*/
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

