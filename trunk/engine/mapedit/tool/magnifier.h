/*	$Csoft: magnifier.h,v 1.3 2003/01/19 12:09:42 vedge Exp $	*/
/*	Public domain	*/

enum magnifier_mode {
	MAGNIFIER_ZOOM_IN,
	MAGNIFIER_ZOOM_OUT,
	MAGNIFIER_CENTER
};

struct magnifier {
	struct tool	tool;
	int		mode;
};

struct magnifier	*magnifier_new(void);
void			 magnifier_init(struct magnifier *);
struct window		*magnifier_window(void *);
void			 magnifier_effect(void *, struct mapview *,
			     Uint32, Uint32);

