/*	$Csoft: stamp.h,v 1.15 2003/06/29 11:33:45 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

#include "begin_code.h"

enum stamp_mode {
	STAMP_REPLACE,
	STAMP_INSERT_HIGHEST
};

struct stamp {
	struct tool	 tool;
	int		 mode;
	struct map	 map;
};

__BEGIN_DECLS
void		 stamp_init(void *);
void		 stamp_destroy(void *);
int		 stamp_cursor(void *, struct mapview *, SDL_Rect *);
void		 stamp_effect(void *, struct mapview *, struct map *,
		              struct node *);
__END_DECLS

#include "close_code.h"
