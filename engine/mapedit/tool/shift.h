/*	$Csoft: shift.h,v 1.7 2003/04/24 01:35:26 vedge Exp $	*/
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
extern DECLSPEC void		 shift_init(void *);
extern DECLSPEC struct window	*shift_window(void *);
extern DECLSPEC void		 shift_effect(void *, struct mapview *,
				              struct node *);
extern DECLSPEC void		 shift_mouse(void *, struct mapview *, Sint16,
				             Sint16);
__END_DECLS

#include "close_code.h"
