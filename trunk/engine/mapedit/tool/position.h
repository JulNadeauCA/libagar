/*	$Csoft: eraser.h,v 1.17 2003/08/29 04:56:06 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

#include "begin_code.h"

struct position {
	struct tool tool;

	struct object	*obj;			/* Object to position */
	int		 dir;			/* Initial direction */
	int		 center;		/* Centering flag */
	struct input	*input;			/* Input device (or NULL) */
};

__BEGIN_DECLS
void	 position_init(void *);
__END_DECLS

#include "close_code.h"
