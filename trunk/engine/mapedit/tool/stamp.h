/*	$Csoft: stamp.h,v 1.10 2003/03/05 02:16:34 vedge Exp $	*/
/*	Public domain	*/

enum stamp_mode {
	STAMP_REPLACE,
	STAMP_INSERT_HIGHEST,
	STAMP_INSERT_INDEX
};

struct stamp {
	struct tool	tool;
	int		mode;
	int		inherit_flags;
};

void		 stamp_init(void *);
struct window	*stamp_window(void *);
int		 stamp_cursor(void *, struct mapview *, SDL_Rect *);
void		 stamp_effect(void *, struct mapview *, struct node *);

