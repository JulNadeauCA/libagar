/*	$Csoft: propedit.h,v 1.8 2003/04/25 09:47:08 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

#include "begin_code.h"

struct propedit {
	struct tool	tool;
	Uint32		node_mode;
	Uint32		node_flags;
};

__BEGIN_DECLS
extern DECLSPEC void		 propedit_init(void *);
extern DECLSPEC struct window	*propedit_window(void *);
extern DECLSPEC void		 propedit_effect(void *, struct mapview *,
				                 struct node *);
extern DECLSPEC int		 propedit_cursor(void *, struct mapview *,
				                 SDL_Rect *);
__END_DECLS

#include "close_code.h"
