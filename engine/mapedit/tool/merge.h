/*	$Csoft: merge.h,v 1.18 2003/09/07 04:17:37 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

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
