/*	$Csoft: propedit.h,v 1.3 2003/01/25 06:29:30 vedge Exp $	*/
/*	Public domain	*/

enum propedit_mode {
	PROPEDIT_CLEAR,
	PROPEDIT_SET,
	PROPEDIT_UNSET
};

struct propedit {
	struct tool	tool;
	int		mode;		/* Mode */
	Uint32		node_mask;	/* Node flags mask */
};

void		 propedit_init(void *);
struct window	*propedit_window(void *);
void		 propedit_effect(void *, struct mapview *, Uint32, Uint32);

