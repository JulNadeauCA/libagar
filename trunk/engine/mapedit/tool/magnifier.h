/*	$Csoft: magnifier.h,v 1.2 2002/07/18 12:02:55 vedge Exp $	*/
/*	Public domain	*/

struct magnifier {
	struct tool	tool;
	enum {
		MAGNIFIER_ZOOM_IN,
		MAGNIFIER_ZOOM_OUT,
		MAGNIFIER_CENTER
	} mode;
};

struct magnifier	*magnifier_new(void);
void			 magnifier_init(struct magnifier *);
struct window		*magnifier_window(void *);
void			 magnifier_effect(void *, struct mapview *,
			     Uint32, Uint32);

