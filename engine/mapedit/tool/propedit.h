/*	$Csoft: propedit.h,v 1.10 2003/05/08 12:27:32 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

#include "begin_code.h"

struct propedit {
	struct tool tool;

	Uint32	node_mode;
	Uint32	node_flags;
	int	origin;
};

__BEGIN_DECLS
void		 propedit_init(void *);
struct window	*propedit_window(void *);
void		 propedit_effect(void *, struct mapview *, struct node *);
int		 propedit_cursor(void *, struct mapview *, SDL_Rect *);
__END_DECLS

#include "close_code.h"
