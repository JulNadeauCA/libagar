/*	$Csoft: propedit.h,v 1.2 2002/07/09 08:25:21 vedge Exp $	*/
/*	Public domain	*/

struct propedit {
	struct	 tool tool;

	enum {
		PROPEDIT_CLEAR,
		PROPEDIT_SET,
		PROPEDIT_UNSET
	} mode;

	Uint32	 flags;
	Uint32	 nodeflags;
};

struct propedit	*propedit_new(struct mapedit *, int);
void		 propedit_init(struct propedit *, struct mapedit *, int);
struct window	*propedit_window(void *);
void		 propedit_effect(void *, struct mapview *, Uint32, Uint32);

