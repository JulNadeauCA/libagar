/*	$Csoft: tool.h,v 1.12 2003/04/25 09:47:08 vedge Exp $	*/
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
	int		  edit;			/* Require MAPVIEW_EDIT? */
	void		(*func)(void *, struct mapview *);
	SLIST_ENTRY(tool_binding) bindings;
};

struct tool {
	struct object	 obj;
	char		*type;
	struct window	*win;			/* Tool settings window */
	struct button	*button;		/* Trigger */
	SDL_Surface	*cursor;		/* Static cursor */
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

#define TOOL_DIALOG_X	16
#define TOOL_DIALOG_Y	205

__BEGIN_DECLS
extern DECLSPEC void	tool_init(struct tool *, char *, const void *);
extern DECLSPEC void	tool_destroy(void *);
extern DECLSPEC void	tool_bind_key(void *, SDLMod, SDLKey,
				      void (*)(void *, struct mapview *), int);

extern DECLSPEC struct mapview	*tool_mapview(void);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_MAPEDIT_TOOL_TOOL_H_ */
