/*	$Csoft: flip.h,v 1.1 2003/03/13 06:19:19 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

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

void		 flip_init(void *);
struct window	*flip_window(void *);
void		 flip_effect(void *, struct mapview *, struct node *);

