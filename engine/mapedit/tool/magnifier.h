/*	$Csoft$	*/
/*	Public domain	*/

struct magnifier {
	struct	 tool tool;
	int	 flags;
#define MAGNIFIER_RESIZE_WINDOW		0x01	/* Allow window resize */
	enum {
		MAGNIFIER_ZOOM_IN,
		MAGNIFIER_ZOOM_OUT,
		MAGNIFIER_CENTER
	} mode;
};

struct magnifier	*magnifier_new(struct mapedit *, int);
void			 magnifier_init(struct magnifier *, struct mapedit *,
			     int);
struct window		*magnifier_window(void *);
void			 magnifier_effect(void *, struct mapview *,
			     Uint32, Uint32);

