/*	$Csoft: magnifier.h,v 1.5 2003/01/26 06:15:21 vedge Exp $	*/
/*	Public domain	*/

enum magnifier_mode {
	MAGNIFIER_ZOOM_IN,
	MAGNIFIER_ZOOM_OUT,
	MAGNIFIER_CENTER
};

struct magnifier {
	struct tool	tool;
	int		mode;
	int		increment;
};

void			 magnifier_init(void *);
struct window		*magnifier_window(void *);
void			 magnifier_effect(void *, struct mapview *,
			     struct node *);

