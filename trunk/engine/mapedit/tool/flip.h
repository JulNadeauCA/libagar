/*	$Csoft: flip.h,v 1.5 2003/06/29 11:33:45 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool.h>

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
void	flip_init(void *);
__END_DECLS

#include "close_code.h"
