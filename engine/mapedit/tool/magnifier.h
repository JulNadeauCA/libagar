/*	$Csoft: magnifier.h,v 1.9 2003/03/25 13:48:05 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

#include "begin_code.h"

struct magnifier {
	struct tool	tool;
	int		increment;
};

__BEGIN_DECLS
extern DECLSPEC void		 magnifier_init(void *);
extern DECLSPEC struct window	*magnifier_window(void *);
extern DECLSPEC void		 magnifier_mouse(void *, struct mapview *,
				                 Sint16, Sint16, Uint8);
__END_DECLS

#include "close_code.h"
