/*	$Csoft: fill.h,v 1.7 2003/07/08 00:34:55 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

#include "begin_code.h"

enum fill_mode {
	FILL_PATTERN,
	FILL_CLEAR
};

struct fill {
	struct tool	tool;
	int		mode;
};

__BEGIN_DECLS
void	 fill_init(void *);
void	 fill_effect(void *, struct mapview *, struct map *, struct node *);
__END_DECLS

#include "close_code.h"
