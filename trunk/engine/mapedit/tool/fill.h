/*	$Csoft: fill.h,v 1.3 2003/03/25 13:48:05 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

#include "begin_code.h"

enum fill_mode {
	FILL_FILL_MAP,
	FILL_CLEAR_MAP
};

struct fill {
	struct tool	tool;
	int		mode;
};

__BEGIN_DECLS
extern DECLSPEC void		 fill_init(void *);
extern DECLSPEC struct window	*fill_window(void *);
extern DECLSPEC void		 fill_effect(void *, struct mapview *,
				             struct node *);
extern DECLSPEC int		 fill_load(void *, int);
extern DECLSPEC int		 fill_save(void *, int);
__END_DECLS

#include "close_code.h"
