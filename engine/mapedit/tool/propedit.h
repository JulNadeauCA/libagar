/*	$Csoft: propedit.h,v 1.2 2003/01/19 12:09:42 vedge Exp $	*/
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

struct propedit	*propedit_new(void);
void		 propedit_init(struct propedit *);
struct window	*propedit_window(void *);
void		 propedit_effect(void *, struct mapview *, Uint32, Uint32);

