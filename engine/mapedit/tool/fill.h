/*	$Csoft: fill.h,v 1.9 2003/02/12 01:09:47 vedge Exp $	*/
/*	Public domain	*/

enum fill_mode {
	FILL_MAP,
	FILL_CLEAR
};

struct fill {
	struct tool	tool;
	int		mode;
};

void		 fill_init(void *);
struct window	*fill_window(void *);
void		 fill_effect(void *, struct mapview *, struct node *);
int		 fill_load(void *, int);
int		 fill_save(void *, int);
