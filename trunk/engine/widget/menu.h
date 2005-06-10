/*	$Csoft: menu.h,v 1.8 2005/05/31 01:32:54 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_MENU_H_
#define _AGAR_WIDGET_MENU_H_

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/toolbar.h>

#include "begin_code.h"

struct AGMenuView;

struct AGMenuItem {
	const char *text;		/* Item label */
	int label;			/* Label surface name */
	int icon;			/* Icon name */
	int state;			/* State (for dynamic items) */

	SDLKey key_equiv;		/* Key shortcut */
	SDLMod key_mod;
	int x, y;			/* Position in parent view */
	struct AGMenuItem *subitems;	/* Child items */
	u_int		  nsubitems;
	
	struct event *onclick;		/* Raised on click */
	struct event *poll;		/* Raised before the item is drawn */

	enum menu_binding {
		MENU_NO_BINDING,
		MENU_INT_BOOL,
		MENU_INT8_BOOL,
		MENU_INT_FLAGS,
		MENU_INT8_FLAGS,
		MENU_INT16_FLAGS,
		MENU_INT32_FLAGS
	} bind_type;
	void		*bind_p;
	Uint32		 bind_flags;
	int		 bind_invert;
	pthread_mutex_t	*bind_lock;

	struct AGMenuView *view;	/* Back pointer to view (subitems) */
	struct AGMenu *pmenu;		/* Parent menu */
	struct AGMenuItem *pitem;	/* Parent item (NULL for top items) */
	struct AGMenuItem *sel_subitem;	/* Selected subitem */
};

struct AGMenu {
	struct widget wid;
	struct AGMenuItem *items;	/* Top-level items */
	u_int		  nitems;
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
	int hspace, vpadding;
	struct timeout submenu_to;
};

__BEGIN_DECLS
struct AGMenu	  *menu_new(void *);
void		   menu_init(struct AGMenu *);
void	 	   menu_scale(void *, int, int);
void		   menu_draw(void *);
void	 	   menu_destroy(void *);

struct AGMenuItem *menu_add_item(struct AGMenu *, const char *);
void		   menu_free_items(struct AGMenu *);
void		   menu_free_subitems(struct AGMenuItem *);

struct window *menu_expand(struct AGMenu *, struct AGMenuItem *, int, int);
void   	       menu_collapse(struct AGMenu *, struct AGMenuItem *);

struct AGMenuItem *menu_dynamic(struct AGMenuItem *, int,
		                void (*)(int, union evarg *),
				const char *, ...);
__inline__ void menu_set_icon(struct AGMenuItem *, SDL_Surface *);
__inline__ void menu_set_label(struct AGMenuItem *, const char *);

struct AGMenuItem *menu_action(struct AGMenuItem *,
		               const char *, int,
			       void (*)(int, union evarg *),
			       const char *, ...);
struct AGMenuItem *menu_action_kb(struct AGMenuItem *,
		                  const char *, int, SDLKey, SDLMod,
			          void (*)(int, union evarg *),
			          const char *, ...);
struct AGMenuItem *menu_tool(struct AGMenuItem *, struct toolbar *,
		             const char *, int, SDLKey, SDLMod,
			     void (*)(int, union evarg *), const char *, ...);
struct AGMenuItem *menu_int_bool_mp(struct AGMenuItem *, const char *, int,
		                    int *, int, pthread_mutex_t *);
struct AGMenuItem *menu_int8_bool_mp(struct AGMenuItem *, const char *, int,
				  Uint8 *, int, pthread_mutex_t *);
struct AGMenuItem *menu_int_flags_mp(struct AGMenuItem *, const char *, int,
			          int *, int, int, pthread_mutex_t *);
struct AGMenuItem *menu_int8_flags_mp(struct AGMenuItem *, const char *,
			            int, Uint8 *, Uint8, int,
				    pthread_mutex_t *);
struct AGMenuItem *menu_int16_flags_mp(struct AGMenuItem *, const char *,
			            int, Uint16 *, Uint16, int,
				    pthread_mutex_t *);
struct AGMenuItem *menu_int32_flags_mp(struct AGMenuItem *, const char *,
			            int, Uint32 *, Uint32, int,
				    pthread_mutex_t *);

#define menu_int_bool(mi,t,i,p,inv) \
	menu_int_bool_mp((mi),(t),(i),(p),(inv),NULL)
#define menu_int8_bool(mi,t,i,p,inv) \
	menu_int8_bool_mp((mi),(t),(i),(p),(inv),NULL)

#define menu_int_flags(mi,t,i,fp,fl,inv) \
	menu_int_flags_mp((mi),(t),(i),(fp),(fl),(inv),NULL)
#define menu_int8_flags(mi,t,i,fp,fl,inv) \
	menu_int8_flags_mp((mi),(t),(i),(fp),(fl),(inv),NULL)
#define menu_int16_flags(mi,t,i,fp,fl,inv) \
	menu_int16_flags_mp((mi),(t),(i),(fp),(fl),(inv),NULL)
#define menu_int32_flags(mi,t,i,fp,fl,inv) \
	menu_int32_flags_mp((mi),(t),(i),(fp),(fl),(inv),NULL)

struct AGMenuItem *menu_separator(struct AGMenuItem *);

void menu_view_init(void *, struct window *, struct AGMenu *,
                    struct AGMenuItem *);
void menu_view_draw(void *);
void menu_view_scale(void *, int, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_MENU_H_ */
