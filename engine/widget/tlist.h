/*	$Csoft: tlist.h,v 1.10 2002/11/15 00:51:35 vedge Exp $	*/
/*	Public domain	*/

#include <engine/widget/scrollbar.h>

struct tlist_item {
	SDL_Surface	*icon;		/* Icon */
	int		 icon_w;
	int		 icon_h;
	char		*text;		/* Label */
	size_t		 text_len;
	void		*p1;		/* User data */
	struct tlist	*tl_bp;		/* Back pointer to widget */
	int		 selected;	/* Item selection */

	TAILQ_ENTRY(tlist_item) items;		/* Items in list */
	TAILQ_ENTRY(tlist_item) selitems;	/* Hack */
};

TAILQ_HEAD(tlist_itemq, tlist_item);

struct tlist {
	struct widget wid;
	
	struct scrollbar *vbar;	/* Vertical scrollbar */

	int	 flags;
#define TLIST_MULTI		0x01	/* Ctrl/shift multiple selections */
#define TLIST_MULTI_STICKY	0x02	/* Sticky multiple selections */
#define TLIST_DROPDOWN		0x04	/* Drop-down menu */
#define TLIST_POLL		0x08	/* Generate a tlist-poll event before
					   each draw and each event. */

	int	 xspacing;	/* Horiz spacing */
	int	 yspacing;	/* Vert spacing */
	int	 item_h;	/* Item height */

	struct tlist_itemq	 items;		/* Current Items */
	struct tlist_itemq	 selitems;	/* Saved items (selected) */
	int			 nitems;
	pthread_mutex_t		 items_lock;
	pthread_mutexattr_t	 items_lockattr;
};

struct tlist		*tlist_new(struct region *, int, int, int);
void			 tlist_init(struct tlist *, int, int, int);
void	 		 tlist_draw(void *);
void	 		 tlist_destroy(void *);

void			 tlist_save_selections(struct tlist *);
void			 tlist_restore_selections(struct tlist *);

void			 tlist_remove_item(struct tlist_item *);
void			 tlist_clear_items(struct tlist *);
struct tlist_item	*tlist_insert_item(struct tlist *,
			     SDL_Surface *, char *, void *);
struct tlist_item	*tlist_insert_item_selected(struct tlist *,
			     SDL_Surface *, char *, void *);

int			 tlist_select(struct tlist_item *);
int			 tlist_unselect(struct tlist_item *);
void			 tlist_unselect_all(struct tlist *);
struct tlist_item	*tlist_item_index(struct tlist *, int);
struct tlist_item	*tlist_item_text(struct tlist *, char *);

