/*	$Csoft: fill.h,v 1.5 2003/06/18 00:47:01 vedge Exp $	*/
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
int		 fill_load(void *, int);
int		 fill_save(void *, int);
__END_DECLS

#include "close_code.h"
