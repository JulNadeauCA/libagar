/*	$Csoft: resize.h,v 1.8 2003/04/24 07:01:46 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

#include "begin_code.h"

struct resize {
	struct tool	tool;
	enum {
		RESIZE_GROW,
		RESIZE_SHRINK
	} mode;
};

__BEGIN_DECLS
extern DECLSPEC void		 resize_init(void *);
extern DECLSPEC struct window	*resize_window(void *);
extern DECLSPEC void		 resize_cursor(void *, struct mapview *,
				               SDL_Rect *);
extern DECLSPEC void		 resize_mouse(void *, struct mapview *, Sint16,
				              Sint16, Uint8);
__END_DECLS

#include "close_code.h"
