/*	$Csoft: magnifier.h,v 1.4 2003/01/25 06:29:30 vedge Exp $	*/
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

void			 magnifier_init(void *);
struct window		*magnifier_window(void *);
void			 magnifier_effect(void *, struct mapview *,
			     Uint32, Uint32);

