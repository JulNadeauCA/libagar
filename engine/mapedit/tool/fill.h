/*	$Csoft: fill.h,v 1.6 2003/06/29 11:33:45 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

#include "begin_code.h"

enum fill_mode {
	FILL_FILL_MAP,
	FILL_CLEAR_MAP
};

struct fill {
	struct tool	tool;
	int		mode;
};

__BEGIN_DECLS
void		 fill_init(void *);
struct window	*fill_window(void *);
void		 fill_effect(void *, struct mapview *, struct map *,
		             struct node *);
__END_DECLS

#include "close_code.h"
