/*	$Csoft: propedit.h,v 1.7 2003/03/25 13:48:05 vedge Exp $	*/
/*	Public domain	*/

#include <engine/mapedit/tool/tool.h>

#include "begin_code.h"

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

__BEGIN_DECLS
extern DECLSPEC void		 propedit_init(void *);
extern DECLSPEC struct window	*propedit_window(void *);
extern DECLSPEC void		 propedit_effect(void *, struct mapview *,
				                 struct node *);
__END_DECLS

#include "close_code.h"
