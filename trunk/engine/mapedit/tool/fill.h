/*	$Csoft: fill.h,v 1.8 2003/08/29 04:56:18 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool.h>

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
