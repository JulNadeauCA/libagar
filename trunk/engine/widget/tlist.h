/*	$Csoft: tlist.h,v 1.48 2005/05/21 05:54:24 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_TLIST_H_
#define _AGAR_WIDGET_TLIST_H_

#include <engine/widget/scrollbar.h>
#include <engine/widget/menu.h>

#include "begin_code.h"

#define TLIST_LABEL_MAX	96

struct tlist_popup {
	const char *iclass;		/* Apply to items of this class */
	struct AGMenu *menu;
	struct AGMenuItem *item;
	struct window *panel;
	TAILQ_ENTRY(tlist_popup) popups;
};

struct tlist_item {
	int	selected;		/* Effective selection */

	SDL_Surface	*iconsrc;		/* Source icon (or NULL) */
	int		 icon;			/* Cached icon surface */
	void		*p1;			/* User-supplied pointer */
	const char	*class;			/* User-supplied class */
	char		 text[TLIST_LABEL_MAX];	/* Label text */
	int		 label;			/* Cached label surface */


	Uint8	 depth;				/* Depth in tree */
	Uint8	 flags;
#define TLIST_VISIBLE_CHILDREN	0x01		/* Child items visible (tree) */
#define TLIST_HAS_CHILDREN	0x02		/* Child items exist (tree) */
	
	TAILQ_ENTRY(tlist_item) items;		/* Items in list */
	TAILQ_ENTRY(tlist_item) selitems;	/* Saved selection hack */
};

TAILQ_HEAD(tlist_itemq, tlist_item);

struct tlist {
	struct widget wid;
	
	int	 flags;
#define TLIST_MULTI		0x01	/* Multiple selections (ctrl/shift) */
#define TLIST_MULTITOGGLE	0x02	/* Multiple toggle-style selections */
#define TLIST_POLL		0x04	/* Generate tlist-poll events */
#define TLIST_TREE		0x10	/* Hack to display trees */

	void	*selected;		/* Default `selected' binding */
	int	 prew, preh;		/* Prescale hint */

	pthread_mutex_t		 lock;
	int	 		 item_h;	/* Item height */
	int			 icon_w;	/* Item icon width */
	void			*dblclicked;	/* Used by double click */
	int			 keymoved;	/* Used by key repeat */
	struct tlist_itemq	 items;		/* Current Items */
	struct tlist_itemq	 selitems;	/* Saved items */
	int			 nitems;	/* Current item count */
	int			 nvisitems;	/* Visible item count */
	struct scrollbar	*sbar;		/* Vertical scrollbar */
	TAILQ_HEAD(,tlist_popup) popups;	/* Popup menus */
	int (*compare_fn)(const struct tlist_item *, const struct tlist_item *);
};

/* Traverse the user pointer of tlist items assumed to be of the same type. */
#define TLIST_FOREACH_ITEM(p, tl, it, type)				\
	for((it) = TAILQ_FIRST(&(tl)->items),				\
	     (p) = (it)!=NULL ? (struct type *)(it)->p1 : NULL;		\
	    (it) != TAILQ_END(&(tl)->children) && (it)->p1 != NULL;	\
	    (it) = TAILQ_NEXT((it), cobjs),				\
	     (p) = (it)!=NULL ? (struct type *)(it)->p1 : NULL)

__BEGIN_DECLS
struct tlist *tlist_new(void *, int);

void	 tlist_init(struct tlist *, int);
void	 tlist_scale(void *, int, int);
void	 tlist_draw(void *);
void	 tlist_destroy(void *);

void		 tlist_prescale(struct tlist *, const char *, int);
void		 tlist_set_item_height(struct tlist *, int);
__inline__ void	 tlist_set_icon(struct tlist *, struct tlist_item *,
	                        SDL_Surface *);
__inline__ int	 tlist_visible_children(struct tlist *, struct tlist_item *);

void			 tlist_remove_item(struct tlist *, struct tlist_item *);
void			 tlist_clear_items(struct tlist *);
void		 	 tlist_restore_selections(struct tlist *);
struct tlist_item	*tlist_insert(struct tlist *, SDL_Surface *,
			              const char *, ...);
struct tlist_item	*tlist_insert_item(struct tlist *, SDL_Surface *,
			                   const char *, void *);
struct tlist_item	*tlist_insert_item_head(struct tlist *, SDL_Surface *,
			                        const char *, void *);
void			 tlist_select(struct tlist *, struct tlist_item *);
void			 tlist_unselect(struct tlist *, struct tlist_item *);
void			 tlist_select_all(struct tlist *);
struct tlist_item	*tlist_select_pointer(struct tlist *, void *);
struct tlist_item	*tlist_select_text(struct tlist *, const char *);
void			 tlist_unselect_all(struct tlist *);
struct tlist_item	*tlist_find_index(struct tlist *, int);
struct tlist_item	*tlist_selected_item(struct tlist *);
void			*tlist_find_pointer(struct tlist *);
struct tlist_item	*tlist_find_text(struct tlist *, const char *);
struct tlist_item	*tlist_first_item(struct tlist *);
struct tlist_item	*tlist_last_item(struct tlist *);

struct AGMenuItem	*tlist_set_popup(struct tlist *, const char *);

void tlist_set_compare_fn(struct tlist *,
    int (*)(const struct tlist_item *, const struct tlist_item *));

int tlist_compare_strings(const struct tlist_item *, const struct tlist_item *);
int tlist_compare_ptrs(const struct tlist_item *, const struct tlist_item *);
int tlist_compare_ptrs_classes(const struct tlist_item *,
			       const struct tlist_item *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_TLIST_H_ */
