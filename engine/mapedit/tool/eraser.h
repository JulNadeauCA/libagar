/*	$Csoft: eraser.h,v 1.16 2003/07/08 00:34:55 vedge Exp $	*/
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
void	 eraser_effect(void *, struct mapview *, struct map *, struct node *);
__END_DECLS

#include "close_code.h"
