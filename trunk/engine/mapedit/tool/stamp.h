/*	$Csoft: stamp.h,v 1.5 2003/01/26 06:15:21 vedge Exp $	*/
/*	Public domain	*/

enum stamp_mode {
	STAMP_REPLACE,
	STAMP_INSERT_HIGHEST
};

struct stamp {
	struct tool	tool;
	int		mode;
};

void		 stamp_init(void *);
struct window	*stamp_window(void *);
void		 stamp_effect(void *, struct mapview *, Uint32, Uint32);

