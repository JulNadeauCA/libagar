/*	$Csoft: select.h,v 1.7 2003/04/25 09:47:08 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

#include "begin_code.h"

struct select {
	struct tool tool;
};

__BEGIN_DECLS
void	 select_init(void *);
__END_DECLS

#include "close_code.h"
