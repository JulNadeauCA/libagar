/*	$Csoft: shift.h,v 1.9 2003/06/18 00:47:01 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

#include "begin_code.h"

enum shift_mode {
	SHIFT_HIGHEST,
	SHIFT_ALL
};

struct shift {
	struct tool	tool;
	int		mode;
	int		multi;
};

__BEGIN_DECLS
void		 shift_init(void *);
struct window	*shift_window(void *);
void		 shift_effect(void *, struct mapview *, struct map *,
		              struct node *);
void		 shift_mouse(void *, struct mapview *, Sint16, Sint16);
__END_DECLS

#include "close_code.h"
