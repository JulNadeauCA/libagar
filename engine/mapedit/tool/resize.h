/*	$Csoft: resize.h,v 1.1 2002/07/18 12:02:35 vedge Exp $	*/
/*	Public domain	*/

struct resize {
	struct tool	tool;
	enum {
		RESIZE_GROW,
		RESIZE_SHRINK
	} mode;
};

struct resize	*resize_new(void);
void		 resize_init(struct resize *);
struct window	*resize_window(void *);
void		 resize_effect(void *, struct mapview *, Uint32, Uint32);

