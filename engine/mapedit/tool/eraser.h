/*	$Csoft: eraser.h,v 1.12 2003/03/25 13:48:05 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

#include "begin_code.h"

enum eraser_mode {
	ERASER_ALL,
	ERASER_HIGHEST
};

struct eraser {
	struct tool	tool;
	int		mode;			/* Eraser mode */
};

__BEGIN_DECLS
extern DECLSPEC void		 eraser_init(void *);
extern DECLSPEC struct window	*eraser_window(void *);
extern DECLSPEC void		 eraser_effect(void *, struct mapview *,
				               struct node *);
extern DECLSPEC int		 eraser_load(void *, int);
extern DECLSPEC int		 eraser_save(void *, int);
__END_DECLS

#include "close_code.h"
