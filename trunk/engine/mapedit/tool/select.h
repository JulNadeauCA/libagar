/*	$Csoft: select.h,v 1.3 2003/02/02 21:14:02 vedge Exp $	*/
/*	Public domain	*/

struct select {
	struct tool	tool;
};

void		 select_init(void *);
struct window	*select_window(void *);

