/*	$Csoft: stamp.h,v 1.3 2003/01/19 12:09:42 vedge Exp $	*/
/*	Public domain	*/

enum stamp_mode {
	STAMP_REPLACE,
	STAMP_INSERT_HIGHEST
};

struct stamp {
	struct tool	tool;
	int		mode;
};

struct stamp	*stamp_new(void);
void		 stamp_init(struct stamp *);
struct window	*stamp_window(void *);
void		 stamp_effect(void *, struct mapview *, Uint32, Uint32);

