/*	$Csoft: tool.h,v 1.14 2003/06/06 02:47:52 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MAPEDIT_TOOL_TOOL_H_
#define _AGAR_MAPEDIT_TOOL_TOOL_H_

#include <engine/map.h>

#include <engine/widget/widget.h>
#include <engine/widget/window.h>
#include <engine/widget/button.h>

#include <engine/mapedit/mapedit.h>
#include <engine/mapedit/mapview.h>

#include "begin_code.h"

struct tool_ops {
	const struct object_ops	obops;

	struct window	*(*window)(void *);
	int		 (*cursor)(void *, struct mapview *, SDL_Rect *);
	void		 (*effect)(void *, struct mapview *, struct node *);
	void		 (*mouse)(void *, struct mapview *, Sint16, Sint16,
			          Uint8);
};

struct tool_binding {
	const char	 *name;
	SDLMod		  mod;
	SDLKey		  key;
	int		  edit;
	void		(*func)(void *, struct mapview *);
	SLIST_ENTRY(tool_binding) bindings;
};

struct tool {
	struct object	 obj;
	char		 type[OBJECT_TYPE_MAX];
	struct window	*win;			/* Tool settings window */
	struct button	*button;		/* Trigger */
	SDL_Surface	*cursor;		/* Static cursor */
	SDL_Surface	*icon;
	SLIST_HEAD(,tool_binding) bindings;
};

enum {
	TOOL_FILL_CURSOR,
	TOOL_MAGNIFIER_CURSOR,
	TOOL_RESIZE_HORIZ_CURSOR,
	TOOL_RESIZE_VERT_CURSOR,
	TOOL_SELECT_CURSOR
};

#define	TOOL(t)		((struct tool *)(t))
#define TOOL_OPS(t)	((struct tool_ops *)OBJECT((t))->ops)

__BEGIN_DECLS
void	tool_init(struct tool *, const char *, const void *);
void	tool_destroy(void *);
void	tool_bind_key(void *, SDLMod, SDLKey,
	              void (*)(void *, struct mapview *), int);
struct mapview	*tool_mapview(void);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_MAPEDIT_TOOL_TOOL_H_ */
