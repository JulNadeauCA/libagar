/*	$Csoft: resize.h,v 1.7 2003/03/25 13:48:05 vedge Exp $	*/
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
void		 resize_cursor(void *, struct mapview *, SDL_Rect *);
void		 resize_mouse(void *, struct mapview *, Sint16, Sint16,
		     Uint8);
