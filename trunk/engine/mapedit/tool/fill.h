/*	$Csoft: fill.h,v 1.1 2003/02/20 04:57:28 vedge Exp $	*/
/*	Public domain	*/

enum fill_mode {
	FILL_FILL_MAP,
	FILL_CLEAR_MAP
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
