/*	$Csoft: position.h,v 1.1 2003/09/07 08:03:14 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

#include "begin_code.h"

struct position {
	struct tool tool;

	int		 center_view;
	struct tlist	*objs_tl;
	struct tlist	*submaps_tl;
	struct tlist	*inputs_tl;
};

__BEGIN_DECLS
void	 position_init(void *);
__END_DECLS

#include "close_code.h"
