/*	$Csoft: shift.h,v 1.5 2003/03/16 02:39:14 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

enum shift_mode {
	SHIFT_HIGHEST,
	SHIFT_ALL
};

struct shift {
	struct tool	tool;
	int		mode;
	int		multi;
};

void		 shift_init(void *);
struct window	*shift_window(void *);
void		 shift_effect(void *, struct mapview *, struct node *);
void		 shift_mouse(void *, struct mapview *, Sint16, Sint16);

