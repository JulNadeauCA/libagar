/*	$Csoft: merge.h,v 1.17 2003/06/29 11:33:45 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool.h>

#include "begin_code.h"

struct merge {
	struct tool tool;

	TAILQ_HEAD(, object)	 brushes;
	struct tlist		*brushes_tl;
};

__BEGIN_DECLS
void	merge_init(void *);
__END_DECLS

#include "close_code.h"
