/*	$Csoft: stamp.h,v 1.16 2003/08/26 07:55:02 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool.h>

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
