/*	$Csoft$	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_MENU_H_
#define _AGAR_WIDGET_MENU_H_

#include <engine/widget/widget.h>
#include <engine/widget/window.h>

#include "begin_code.h"

struct AGMenuItem {
	const char *text;
	SDL_Surface *surface;
	SDLKey key_equiv;
	SDLMod key_mod;

	void (*fn)(void *arg);
	void *arg;

	struct AGMenuItem *subitems;
	unsigned int	  nsubitems;
};

struct AGMenu {
	struct widget wid;

	struct AGMenuItem *items;	/* Top-level items */
	unsigned int	  nitems;
	
	int vspace;			/* Vertical spacing */
	int hspace;			/* Horizontal spacing */
};

__BEGIN_DECLS
struct AGMenu	  *ag_menu_new(void *);
void		   ag_menu_init(struct AGMenu *);
void	 	   ag_menu_scale(void *, int, int);
void		   ag_menu_draw(void *);
void	 	   ag_menu_destroy(void *);
struct AGMenuItem *ag_menu_add_item(struct AGMenu *, const char *);
struct AGMenuItem *ag_menu_add_subitem(struct AGMenuItem *, const char *,
		                      SDL_Surface *, SDLKey, SDLMod,
				      void (*)(void *), void *);
void		   ag_menu_free_items(struct AGMenu *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_MENU_H_ */
