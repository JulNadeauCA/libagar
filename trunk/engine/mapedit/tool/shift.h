/*	$Csoft: shift.h,v 1.11 2003/09/07 04:17:37 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

#include "begin_code.h"

enum shift_mode {
	SHIFT_HIGHEST,
	SHIFT_ALL
};

struct shift {
	struct tool tool;

	int	 mode;			/* Shift method */
	int	 multi;			/* Apply to all selected nodes */
};

__BEGIN_DECLS
void	shift_init(void *);
void	shift_mouse(void *, struct mapview *, Sint16, Sint16);
__END_DECLS

#include "close_code.h"
