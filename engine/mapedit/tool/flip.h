/*	$Csoft: select.h,v 1.4 2003/03/11 02:46:53 vedge Exp $	*/
/*	Public domain	*/

enum flip_mode {
	FLIP_HORIZ,
	FLIP_VERT
};

struct flip {
	struct tool	tool;
	int		mode;
};

void		 flip_init(void *);
struct window	*flip_window(void *);
void		 flip_effect(void *, struct mapview *, struct node *);

