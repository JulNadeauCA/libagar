/*	$Csoft: select.h,v 1.2 2003/01/26 06:15:21 vedge Exp $	*/
/*	Public domain	*/

struct select {
	struct tool	tool;
};

void		 select_init(void *);
struct window	*select_window(void *);
void		 select_effect(void *, struct mapview *, struct node *);

