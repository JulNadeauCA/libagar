/*	$Csoft: magnifier.h,v 1.11 2003/06/18 00:47:01 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool.h>

#include "begin_code.h"

struct magnifier {
	struct tool tool;

	int	increment;
};

__BEGIN_DECLS
void	magnifier_init(void *);
__END_DECLS

#include "close_code.h"
