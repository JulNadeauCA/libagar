/*	$Csoft: flip.h,v 1.2 2003/03/25 13:48:05 vedge Exp $	*/
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
extern DECLSPEC void		 flip_init(void *);
extern DECLSPEC struct window	*flip_window(void *);
extern DECLSPEC void		 flip_effect(void *, struct mapview *,
				             struct node *);
__END_DECLS

#include "close_code.h"
