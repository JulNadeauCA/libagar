/*	$Csoft: magnifier.h,v 1.12 2003/09/07 04:17:37 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

#include "begin_code.h"

struct magnifier {
	struct tool tool;

	int	increment;
};

__BEGIN_DECLS
void	magnifier_init(void *);
__END_DECLS

#include "close_code.h"
