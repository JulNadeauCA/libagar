/*	$Csoft: tlist.h,v 1.1 2002/09/06 01:27:34 vedge Exp $	*/
/*	Public domain	*/

struct tlist_item {
	SDL_Surface	*icon;		/* Icon */
	int		 icon_w;
	int		 icon_h;
	char		*text;		/* Label */
	size_t		 text_len;
	void		*p1;		/* User data */
	struct tlist	*tl_bp;		/* Back pointer to widget */

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
		int	soft_start;
		int	start;
		int	sel;
	} offs;

	struct {
		void	(*update)(struct tlist *);	/* Before redraw */
	} ops;

	TAILQ_HEAD(, tlist_item) items;
	int			 nitems;
	pthread_mutex_t		 items_lock;
};

struct tlist	*tlist_new(struct region *, int, int, int);
void		 tlist_init(struct tlist *, int, int, int);
void	 	 tlist_draw(void *);

void			 tlist_remove_item(struct tlist_item *);
void			 tlist_clear_items(struct tlist *);
struct tlist_item	*tlist_insert_item(struct tlist *, SDL_Surface *,
			     char *, void *);

