/*	$Csoft: magnifier.h,v 1.6 2003/02/02 21:14:02 vedge Exp $	*/
/*	Public domain	*/

enum magnifier_mode {
	MAGNIFIER_ZOOM_IN,
	MAGNIFIER_ZOOM_OUT
};

struct magnifier {
	struct tool	tool;
	int		mode;
	int		increment;
};

void		 magnifier_init(void *);
struct window	*magnifier_window(void *);
void		 magnifier_effect(void *, struct mapview *, struct node *);
void		 magnifier_mouse(void *, struct mapview *, Sint16, Sint16,
		     Uint8);

