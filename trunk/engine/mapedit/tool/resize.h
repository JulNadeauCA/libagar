/*	$Csoft: resize.h,v 1.9 2003/04/25 09:47:08 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

#include "begin_code.h"

struct resize {
	struct tool tool;

	enum {
		RESIZE_GROW,
		RESIZE_SHRINK
	} mode;
};

__BEGIN_DECLS
void		 resize_init(void *);
struct window	*resize_window(void *);
void		 resize_cursor(void *, struct mapview *, SDL_Rect *);
void		 resize_mouse(void *, struct mapview *, Sint16, Sint16, Uint8);
__END_DECLS

#include "close_code.h"
