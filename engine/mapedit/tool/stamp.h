/*	$Csoft: stamp.h,v 1.6 2003/01/29 00:35:29 vedge Exp $	*/
/*	Public domain	*/

enum stamp_mode {
	STAMP_REPLACE,
	STAMP_INSERT_HIGHEST,
	STAMP_INSERT_INDEX
};

struct stamp {
	struct tool	tool;
	int		mode;
};

void		 stamp_init(void *);
struct window	*stamp_window(void *);
void		 stamp_effect(void *, struct mapview *, struct node *);

