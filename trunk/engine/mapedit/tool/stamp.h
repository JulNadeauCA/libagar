/*	$Csoft: stamp.h,v 1.13 2003/04/25 09:47:08 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

#include "begin_code.h"

enum stamp_mode {
	STAMP_REPLACE,
	STAMP_INSERT_HIGHEST,
	STAMP_INSERT_INDEX
};

struct stamp {
	struct tool	tool;
	int		mode;
	int		inherit_flags;
};

__BEGIN_DECLS
void		 stamp_init(void *);
struct window	*stamp_window(void *);
int		 stamp_cursor(void *, struct mapview *, SDL_Rect *);
void		 stamp_effect(void *, struct mapview *, struct node *);
__END_DECLS

#include "close_code.h"
