/*	$Csoft: flip.h,v 1.4 2003/06/18 00:47:01 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

#include "begin_code.h"

enum flip_mode {
	FLIP_HORIZ,
	FLIP_VERT
};

enum flip_which {
	FLIP_ALL,
	FLIP_HIGHEST
};

struct flip {
	struct tool	tool;
	int		mode;
	int		which;
};

__BEGIN_DECLS
void		 flip_init(void *);
struct window	*flip_window(void *);
void		 flip_effect(void *, struct mapview *, struct map *,
		             struct node *);
__END_DECLS

#include "close_code.h"
