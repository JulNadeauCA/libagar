/*	$Csoft: tlist.h,v 1.29 2003/06/13 02:48:42 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_TLIST_H_
#define _AGAR_WIDGET_TLIST_H_

#include <engine/widget/scrollbar.h>

#include "begin_code.h"

#define TLIST_LABEL_MAX	64

struct tlist_item {
	SDL_Surface	*icon;			/* Original icon */
	char		 text[TLIST_LABEL_MAX];	/* Label */
	size_t		 text_len;		/* Label size (optimization) */
	void		*p1;			/* User data */
	int		 selected;		/* Item selection */
	int		 vischilds, haschilds;	/* Hack for displaying trees */
	int		 depth;			/* Depth in tree */

	TAILQ_ENTRY(tlist_item) items;		/* Items in list */
	TAILQ_ENTRY(tlist_item) selitems;	/* Hack */
};

TAILQ_HEAD(tlist_itemq, tlist_item);

struct tlist {
	struct widget wid;
	
	int	 flags;
#define TLIST_MULTI		0x01	/* Ctrl/shift multiple selections */
#define TLIST_MULTI_STICKY	0x02	/* Sticky multiple selections */
#define TLIST_POLL		0x04	/* Generate tlist-poll events */
#define TLIST_DBLCLICK		0x08	/* Generate tlist-dblclick events */
#define TLIST_TREE		0x10	/* Hack to display trees */

	int	 prew, preh;		/* Prescale hint */

	pthread_mutex_t		 lock;
	int	 		 item_h;	/* Item height */
	struct tlist_item	*dblclicked;	/* Last clicked on this item */
	SDL_TimerID		 dbltimer;	/* Double click timer */
	struct tlist_itemq	 items;		/* Current Items */
	struct tlist_itemq	 selitems;	/* Saved items */
	int			 nitems;	/* Current item count */
	int			 nvisitems;	/* Visible item count */
	struct scrollbar	*sbar;		/* Vertical scrollbar */
};

__BEGIN_DECLS
struct tlist *tlist_new(void *, int);

void	 tlist_init(struct tlist *, int);
void	 tlist_scale(void *, int, int);
void	 tlist_draw(void *);
void	 tlist_destroy(void *);

void	 tlist_prescale(struct tlist *, const char *, int);
void	 tlist_set_item_height(struct tlist *, int);
void	 tlist_set_item_icon(struct tlist *, struct tlist_item *,
	                     SDL_Surface *);
void	 tlist_save_selections(struct tlist *);
void	 tlist_restore_selections(struct tlist *);
int	 tlist_visible_childs(struct tlist *, struct tlist_item *);
void	 tlist_scroll(struct tlist *, int);

void			 tlist_remove_item(struct tlist *, struct tlist_item *);
void			 tlist_clear_items(struct tlist *);
struct tlist_item	*tlist_insert_item(struct tlist *, SDL_Surface *,
			                   const char *, const void *);
struct tlist_item	*tlist_insert_item_head(struct tlist *, SDL_Surface *,
			                        const char *, const void *);
int			 tlist_select(struct tlist *, struct tlist_item *);
int			 tlist_unselect(struct tlist *, struct tlist_item *);
void			 tlist_select_all(struct tlist *);
void			 tlist_unselect_all(struct tlist *);
struct tlist_item	*tlist_item_index(struct tlist *, int);
struct tlist_item	*tlist_item_selected(struct tlist *);
struct tlist_item	*tlist_item_text(struct tlist *, const char *);
struct tlist_item	*tlist_item_first(struct tlist *);
struct tlist_item	*tlist_item_last(struct tlist *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_TLIST_H_ */
