/*	$Csoft: select.h,v 1.4 2003/01/19 12:09:42 vedge Exp $	*/
/*	Public domain	*/

struct select {
	struct tool	tool;
};

struct select	*select_new(void);
void		 select_init(struct select *);
struct window	*select_window(void *);
void		 select_effect(void *, struct mapview *, Uint32, Uint32);

