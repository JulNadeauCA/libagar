/*	$Csoft: fill.h,v 1.2 2003/03/07 03:24:49 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

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
