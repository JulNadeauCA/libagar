/*	$Csoft: propedit.h,v 1.12 2003/06/29 11:33:45 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool.h>

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
