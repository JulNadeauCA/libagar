/*	$Csoft: stamp.h,v 1.4 2003/01/25 06:29:30 vedge Exp $	*/
/*	Public domain	*/

enum stamp_mode {
	STAMP_REPLACE,
	STAMP_INSERT_HIGHEST
};

struct stamp {
	struct tool	tool;
	int		mode;

	TAILQ_HEAD(, map) brushes;
};

void		 stamp_init(void *);
struct window	*stamp_window(void *);
void		 stamp_effect(void *, struct mapview *, Uint32, Uint32);

