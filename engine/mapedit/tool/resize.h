/*	$Csoft: resize.h,v 1.11 2003/09/07 04:17:37 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

#include "begin_code.h"

struct resize {
	struct tool tool;
};

__BEGIN_DECLS
void	resize_init(void *);
__END_DECLS

#include "close_code.h"
