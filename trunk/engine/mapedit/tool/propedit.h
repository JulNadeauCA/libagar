/*	$Csoft: propedit.h,v 1.1 2002/07/30 22:19:34 vedge Exp $	*/
/*	Public domain	*/

struct propedit {
	struct tool	tool;
	enum {
		PROPEDIT_CLEAR,
		PROPEDIT_SET,
		PROPEDIT_UNSET
	} mode;
	Uint32	nodeflags;
};

struct propedit	*propedit_new(void);
void		 propedit_init(struct propedit *);
struct window	*propedit_window(void *);
void		 propedit_effect(void *, struct mapview *, Uint32, Uint32);

