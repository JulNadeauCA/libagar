/*	$Csoft: magnifier.h,v 1.7 2003/02/22 11:44:05 vedge Exp $	*/
/*	Public domain	*/

struct magnifier {
	struct tool	tool;
	int		increment;
};

void		 magnifier_init(void *);
struct window	*magnifier_window(void *);
void		 magnifier_mouse(void *, struct mapview *, Sint16, Sint16,
		     Uint8);

