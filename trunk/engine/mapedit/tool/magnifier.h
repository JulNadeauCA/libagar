/*	$Csoft: magnifier.h,v 1.10 2003/04/25 09:47:08 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

#include "begin_code.h"

struct magnifier {
	struct tool tool;

	int	increment;
};

__BEGIN_DECLS
void		 magnifier_init(void *);
struct window	*magnifier_window(void *);
void		 magnifier_mouse(void *, struct mapview *, Sint16, Sint16,
		                 Uint8);
__END_DECLS

#include "close_code.h"
