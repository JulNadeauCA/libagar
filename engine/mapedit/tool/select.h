/*	$Csoft: select.h,v 1.8 2003/06/18 00:47:01 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool.h>

#include "begin_code.h"

struct select {
	struct tool tool;
};

__BEGIN_DECLS
void	 select_init(void *);
__END_DECLS

#include "close_code.h"
