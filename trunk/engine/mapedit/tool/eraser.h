/*	$Csoft: eraser.h,v 1.17 2003/08/29 04:56:06 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool.h>

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
