/*	$Csoft: eraser.h,v 1.18 2003/09/07 04:17:36 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

#include "begin_code.h"

enum eraser_mode {
	ERASER_ALL,
	ERASER_HIGHEST
};

struct eraser {
	struct tool	tool;
	int		mode;			/* Eraser mode */
};

__BEGIN_DECLS
void	 eraser_init(void *);
__END_DECLS

#include "close_code.h"
