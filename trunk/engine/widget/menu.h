/*	$Csoft: menu.h,v 1.1 2004/09/12 05:51:10 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_MENU_H_
#define _AGAR_WIDGET_MENU_H_

#include <engine/widget/widget.h>
#include <engine/widget/window.h>

#include "begin_code.h"

struct AGMenuView;

struct AGMenuItem {
	const char *text;		/* Item label */
	int label;			/* Label surface name */
	int icon;			/* Icon name */
	SDLKey key_equiv;		/* Key shortcut */
	SDLMod key_mod;
	int x, y;			/* Position in parent view */

	struct AGMenuItem *subitems;	/* Child items */
	unsigned int nsubitems;

	struct AGMenuView *view;	/* Back pointer to view (subitems) */
	struct event *event;		/* Event to raise */
	struct AGMenu *pmenu;		/* Parent menu */
	struct AGMenuItem *pitem;	/* Parent item (NULL for top items) */
	struct AGMenuItem *sel_subitem;	/* Selected subitem */
};

struct AGMenu {
	struct widget wid;
	struct AGMenuItem *items;	/* Top-level items */
	unsigned int nitems;
	int selecting;			/* Selection in progress */
	struct AGMenuItem *sel_item;	/* Selected top-level item */
	int hspace, vspace;		/* Spacing */
	int itemh;			/* Item height (optimization) */
};

struct AGMenuView {
	struct widget wid;
	struct window *panel;
	struct AGMenu *pmenu;
	struct AGMenuItem *pitem;
	int hspace, vspace;		/* Spacing */
	struct timeout submenu_to;
};

__BEGIN_DECLS
struct AGMenu	  *ag_menu_new(void *);
void		   ag_menu_init(struct AGMenu *);
void	 	   ag_menu_scale(void *, int, int);
void		   ag_menu_draw(void *);
void	 	   ag_menu_destroy(void *);
struct AGMenuItem *ag_menu_add_item(struct AGMenu *, const char *);
struct AGMenuItem *ag_menu_add_subitem(struct AGMenuItem *,
		                       const char *, SDL_Surface *, SDLKey,
				       SDLMod, void (*)(int, union evarg *),
				       const char *, ...);
void		   ag_menu_free_items(struct AGMenu *);
void		   ag_menu_free_subitems(struct AGMenuItem *);
void   		   ag_menu_collapse(struct AGMenu *, struct AGMenuItem *);
void		   ag_menu_expand(struct AGMenu *, struct AGMenuItem *, int,
		                  int);

void   ag_menu_view_init(void *, struct window *, struct AGMenu *,
	                 struct AGMenuItem *);
void   ag_menu_view_draw(void *);
void   ag_menu_view_scale(void *, int, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_MENU_H_ */
