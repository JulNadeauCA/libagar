/*	$Csoft: plist.h,v 1.1 2004/05/15 04:05:12 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_PLIST_H_
#define _AGAR_WIDGET_PLIST_H_

#include <engine/widget/scrollbar.h>

#include "begin_code.h"

#define PLIST_LABEL_MAX	64

struct plist_item {
	SDL_Surface	*icon;			/* Icon surface (shared) */
	void		*p;			/* User-supplied pointer */
	SDL_Surface	*label;			/* Rasterized text */
	char		 text[PLIST_LABEL_MAX];	/* Label text */
};

struct plist {
	struct widget wid;

	int	 flags;
#define PLIST_DBLCLICK		0x01	/* Generate plist-dblclick events */

	void	*selected;		/* Default `selected' binding */
	int	 prew, preh;		/* Prescale hint */

	pthread_mutex_t	lock;

	int	item_h;			/* Item height */
	int	dblclicked;		/* Used by double click */
	int	keymoved;		/* Used by key repeat */

	struct plist_item *items;	/* Item array */
	int	       *selitems;	/* Selected item array */
	unsigned int	  nitems;	/* Current items */
	unsigned int	maxitems;	/* Allocated items */
	unsigned int   nvisitems;	/* Visible items */

	struct scrollbar *sbar;		/* Vertical scrollbar */
};

__BEGIN_DECLS
struct plist	*plist_new(void *, int);
void	 	 plist_init(struct plist *, int);
void	 	 plist_scale(void *, int, int);
void	 	 plist_draw(void *);
void	 	 plist_destroy(void *);
void		 plist_prescale(struct plist *, const char *, int);

struct plist_item *plist_insert(struct plist *, SDL_Surface *, const char *,
		                void *);
void		   plist_clear(struct plist *);
void		   plist_set_item_height(struct plist *, int);
__inline__ void	   plist_set_icon(struct plist *, struct plist_item *,
	                          SDL_Surface *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_PLIST_H_ */
