/*	$Csoft: select.h,v 1.1 2003/01/24 08:27:05 vedge Exp $	*/
/*	Public domain	*/

struct select {
	struct tool	tool;
};

void		 select_init(void *);
struct window	*select_window(void *);
void		 select_effect(void *, struct mapview *, Uint32, Uint32);

