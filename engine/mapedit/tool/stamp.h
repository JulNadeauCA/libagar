/*	$Csoft: stamp.h,v 1.17 2003/09/07 04:17:37 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

#include "begin_code.h"

enum stamp_mode {
	STAMP_REPLACE,
	STAMP_INSERT_HIGHEST
};

struct stamp {
	struct tool tool;

	int	mode;				/* Mode of operation */
};

__BEGIN_DECLS
void	stamp_init(void *);
__END_DECLS

#include "close_code.h"
