/*	$Csoft: tlist.h,v 1.3 2002/07/29 05:29:29 vedge Exp $	*/
/*	Public domain	*/

struct tlist_item {
	SDL_Surface *icon;		/* Icon */
	enum {
		TLIST_ITEM_ICON_LEFT,
		TLIST_ITEM_ICON_RIGHT
	} icon_align;

	char	*text;			/* Label */
	size_t	 text_len;

	void	*p1;			/* User data */

	TAILQ_ENTRY(tlist_item) items;
};

struct tlist {
	struct widget wid;

	int	 flags;
#define TLIST_DROPDOWN	0x01	/* Drop-down menu */

	int	 xspacing;	/* Horiz spacing */
	int	 yspacing;	/* Vert spacing */
	int	 item_h;	/* Item height */

	struct {
		void	(*update)(struct tlist *);	/* Before redraw */
	} ops;

	TAILQ_HEAD(, tlist_item) items;
};

struct tlist	*tlist_new(struct region *, int, int, int);
void		 tlist_init(struct tlist *, int, int, int);
void	 	 tlist_draw(void *);

