/*	$Csoft: propedit.h,v 1.6 2003/02/10 23:05:19 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

enum propedit_mode {
	PROPEDIT_CLEAR,
	PROPEDIT_SET,
	PROPEDIT_UNSET
};

struct propedit {
	struct tool	tool;
	int		mode;		/* Mode */
	Uint32		node_mode;
	Uint32		node_mask;	/* Node flags mask */
};

void		 propedit_init(void *);
struct window	*propedit_window(void *);
void		 propedit_effect(void *, struct mapview *, struct node *);

