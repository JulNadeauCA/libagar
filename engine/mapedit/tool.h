/*	$Csoft$	*/
/*	Public domain	*/

#ifndef _AGAR_MAPEDIT_TOOL_H_
#define _AGAR_MAPEDIT_TOOL_H_
#include "begin_code.h"

struct mapview;
struct tool_kbinding;

struct tool {
	const char *name;			/* Name of the tool */
	const char *desc;			/* Short description */
	int icon;				/* Icon (-1 = none) */
	int cursor_index;			/* Cursor (-1 = none) */

	void (*init)(void *);
	void (*destroy)(void *);
	int  (*load)(void *, struct netbuf *);
	int  (*save)(void *, struct netbuf *);
	void (*effect)(void *, struct mapview *, struct map *, struct node *);
	int  (*cursor)(void *, struct mapview *, SDL_Rect *);
	void (*mouse)(void *, struct mapview *, Sint16, Sint16, Uint8);

	struct mapview *mv;
	struct window *win;
	struct button *trigger;
	SDL_Surface *cursor_su;			/* Static cursor surface */
	SLIST_HEAD(,tool_kbinding) kbindings;	/* Keyboard bindings */
	TAILQ_ENTRY(tool) tools;
};

struct tool_kbinding {
	const char *name;			/* Description */
	SDLMod mod;				/* Key modifier */
	SDLKey key;				/* Key */
	int edit;				/* Require edition mode */
	void (*func)(struct mapview *);		/* Callback function */
	SLIST_ENTRY(tool_kbinding) kbindings;
};

#define	TOOL(t)	((struct tool *)(t))

__BEGIN_DECLS
void		 tool_init(struct tool *, struct mapview *);
void		 tool_destroy(struct tool *);
struct window	*tool_window(void *, const char *);
void		 tool_bind_key(void *, SDLMod, SDLKey,
		               void (*)(struct mapview *), int);
void		 tool_unbind_key(void *, SDLMod, SDLKey);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_MAPEDIT_TOOL_H_ */
