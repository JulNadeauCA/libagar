/*	$Csoft: magnifier.h,v 1.8 2003/03/16 02:58:20 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

struct magnifier {
	struct tool	tool;
	int		increment;
};

void		 magnifier_init(void *);
struct window	*magnifier_window(void *);
void		 magnifier_mouse(void *, struct mapview *, Sint16, Sint16,
		     Uint8);

