/*	$Csoft: tlist.h,v 1.21 2003/05/04 01:48:38 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_TLIST_H_
#define _AGAR_WIDGET_TLIST_H_

#include <engine/widget/scrollbar.h>

#include "begin_code.h"

struct tlist_item {
	SDL_Surface	*icon;		/* Original icon */
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
	struct widget	wid;
	
	int	 flags;
#define TLIST_MULTI		0x01	/* Ctrl/shift multiple selections */
#define TLIST_MULTI_STICKY	0x02	/* Sticky multiple selections */
#define TLIST_DROPDOWN		0x04	/* Drop-down menu */
#define TLIST_POLL		0x08	/* Generate tlist-poll events */
#define TLIST_DBLCLICK		0x10	/* Generate tlist-dblclick events */

	int	 		 item_h;	/* Item height */
	struct scrollbar	 sbar;		/* Scrollbar */

	pthread_mutex_t		 lock;
	struct tlist_item	*dblclicked;	/* Last clicked on this item */
	SDL_TimerID		 dbltimer;	/* Double click timer */
	struct tlist_itemq	 items;		/* Current Items */
	struct tlist_itemq	 selitems;	/* Saved items */
	int			 nitems;	/* Current item count */
	int			 nvisitems;	/* Visible item count */
};

__BEGIN_DECLS
extern DECLSPEC struct tlist	*tlist_new(struct region *, int, int, int);
extern DECLSPEC void		 tlist_init(struct tlist *, int, int, int);
extern DECLSPEC void	 	 tlist_draw(void *);
extern DECLSPEC void	 	 tlist_destroy(void *);
extern DECLSPEC void		 tlist_set_item_height(struct tlist *, int);
extern DECLSPEC void		 tlist_set_item_icon(struct tlist *,
				                     struct tlist_item *,
						     SDL_Surface *);
extern DECLSPEC void		 tlist_save_selections(struct tlist *);
extern DECLSPEC void		 tlist_restore_selections(struct tlist *);
extern DECLSPEC void		 tlist_scroll(struct tlist *, int);

extern DECLSPEC void			 tlist_remove_item(struct tlist_item *);
extern DECLSPEC void			 tlist_clear_items(struct tlist *);
extern DECLSPEC struct tlist_item	*tlist_insert_item(struct tlist *,
					                   SDL_Surface *,
							   char *, void *);
extern DECLSPEC struct tlist_item	*tlist_insert_item_head(struct tlist *,
					                        SDL_Surface *,
								char *, void *);

extern DECLSPEC int			 tlist_select(struct tlist_item *);
extern DECLSPEC int			 tlist_unselect(struct tlist_item *);
extern DECLSPEC void			 tlist_select_all(struct tlist *);
extern DECLSPEC void			 tlist_unselect_all(struct tlist *);
extern DECLSPEC struct tlist_item	*tlist_item_index(struct tlist *, int);
extern DECLSPEC struct tlist_item	*tlist_item_selected(struct tlist *);
extern DECLSPEC struct tlist_item	*tlist_item_text(struct tlist *,
					                 char *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_TLIST_H_ */
