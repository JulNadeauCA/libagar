/*	$Csoft: propedit.h,v 1.13 2003/09/07 04:17:37 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

#include "begin_code.h"

struct propedit {
	struct tool tool;

	Uint32	node_mode;			/* Set type of reference */
	Uint32	node_flags;			/* Set reference flags */
	int	origin;				/* Set origin point */
};

__BEGIN_DECLS
void	propedit_init(void *);
__END_DECLS

#include "close_code.h"
