/*	$Csoft: stamp.h,v 1.12 2003/03/25 13:48:05 vedge Exp $	*/
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
extern DECLSPEC void		 stamp_init(void *);
extern DECLSPEC struct window	*stamp_window(void *);
extern DECLSPEC int		 stamp_cursor(void *, struct mapview *,
				              SDL_Rect *);
extern DECLSPEC void		 stamp_effect(void *, struct mapview *,
				              struct node *);
__END_DECLS

#include "close_code.h"
