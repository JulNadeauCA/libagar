/*	$Csoft: resize.h,v 1.1 2002/07/09 09:23:58 vedge Exp $	*/
/*	Public domain	*/

struct resize {
	struct	 tool tool;
	int	 flags;
#define RESIZE_CAN_RESIZE_WIN		0x01	/* Allow window resize */
	enum {
		RESIZE_GROW,
		RESIZE_SHRINK
	} mode;
};

struct resize	*resize_new(struct mapedit *, int);
void		 resize_init(struct resize *, struct mapedit *, int);
struct window	*resize_window(void *);
void		 resize_effect(void *, struct mapview *, Uint32, Uint32);

