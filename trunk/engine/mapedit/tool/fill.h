/*	$Csoft: fill.h,v 1.9 2003/09/07 04:17:36 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

#include "begin_code.h"

enum fill_mode {
	FILL_PATTERN,
	FILL_CLEAR
};

struct fill {
	struct tool	tool;
	int		mode;
};

__BEGIN_DECLS
void	 fill_init(void *);
__END_DECLS

#include "close_code.h"
