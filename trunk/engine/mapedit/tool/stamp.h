/*	$Csoft: stamp.h,v 1.2 2002/07/09 08:25:21 vedge Exp $	*/
/*	Public domain	*/

struct stamp {
	struct tool	tool;
	enum {
		STAMP_REPLACE,
		STAMP_INSERT_HIGHEST,
	} mode;
};

struct stamp	*stamp_new(void);
void		 stamp_init(struct stamp *);
struct window	*stamp_window(void *);
void		 stamp_effect(void *, struct mapview *, Uint32, Uint32);

