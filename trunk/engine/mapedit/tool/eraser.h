/*	$Csoft: eraser.h,v 1.13 2003/04/25 09:47:08 vedge Exp $	*/
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
void		 eraser_init(void *);
struct window	*eraser_window(void *);
void		 eraser_effect(void *, struct mapview *, struct node *);
int		 eraser_load(void *, int);
int		 eraser_save(void *, int);
__END_DECLS

#include "close_code.h"
