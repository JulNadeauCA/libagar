/*	$Csoft: resize.h,v 1.6 2003/03/13 08:27:01 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

struct resize {
	struct tool	tool;
	enum {
		RESIZE_GROW,
		RESIZE_SHRINK
	} mode;
};

void		 resize_init(void *);
struct window	*resize_window(void *);
void		 resize_mouse(void *, struct mapview *, Sint16, Sint16,
		     Uint8);

