/*	$Csoft: mapedit.h,v 1.93 2004/03/10 16:58:35 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MAPEDIT_H_
#define _AGAR_MAPEDIT_H_

#include <engine/mapedit/mapview.h>

#include <engine/widget/window.h>
#include <engine/widget/button.h>

#include "begin_code.h"

struct tool_kbinding {
	const char *name;			/* Description */
	SDLMod mod;				/* Key modifier */
	SDLKey key;				/* Key */
	int edit;				/* Require edition mode */
	void (*func)(struct mapview *);		/* Callback function */
	SLIST_ENTRY(tool_kbinding) kbindings;
};

struct tool {
	const char *name;			/* Name of the tool */
	const char *desc;			/* Short description */
	int icon;				/* Icon (-1 = none) */
	int cursor_index;			/* Cursor (-1 = none) */

	void (*init)(void);
	void (*destroy)(void);
	int  (*load)(struct netbuf *);
	int  (*save)(struct netbuf *);
	void (*effect)(struct mapview *, struct map *, struct node *);
	int  (*cursor)(struct mapview *, SDL_Rect *);
	void (*mouse)(struct mapview *, Sint16, Sint16, Uint8);

	struct window *win;			/* Tool settings window */
	struct button *trigger;
	SDL_Surface *cursor_su;			/* Static cursor surface */
	SLIST_HEAD(,tool_kbinding) kbindings;	/* Keyboard bindings */
};

struct mapedit {
	struct object obj;
	struct tool *curtool;		/* Selected tool */
	struct map copybuf;		/* Copy/paste buffer */
	struct object pseudo;		/* Pseudo-object (for depkeeping) */
};

#define	TOOL(t)	((struct tool *)(t))

enum {
	/* Tool icons */
	MAPEDIT_TOOL_OBJEDITOR,
	MAPEDIT_TOOL_STAMP,
	MAPEDIT_TOOL_ERASER,
	MAPEDIT_TOOL_NEW_VIEW,
	MAPEDIT_TOOL_MAGNIFIER,
	MAPEDIT_TOOL_RESIZE,
	MAPEDIT_TOOL_GRID,
	MAPEDIT_TOOL_PROPS,
	MAPEDIT_TOOL_PROPEDIT,
	MAPEDIT_TOOL_EDIT,
	MAPEDIT_TOOL_RIGHT,
	MAPEDIT_TOOL_LEFT,
	MAPEDIT_TOOL_UP,
	MAPEDIT_TOOL_DOWN,
	MAPEDIT_TOOL_NODEEDIT,
	MAPEDIT_TOOL_LAYEDIT,
	MAPEDIT_TOOL_SELECT,
	MAPEDIT_TOOL_SHIFT,
	MAPEDIT_TOOL_MERGE,
	MAPEDIT_TOOL_FILL,
	MAPEDIT_TOOL_FLIP,
	MAPEDIT_TOOL_MIMPORT,
	MAPEDIT_TOOL_POSITION,
	MAPEDIT_TOOL_INVERT,

	/* Tool cursors */
	MAPEDIT_SELECT_CURSOR,
	MAPEDIT_FILL_CURSOR,
	MAPEDIT_MAGNIFIER_CURSOR,
	MAPEDIT_RESIZE_V_CURSOR,
	MAPEDIT_RESIZE_H_CURSOR,
	MAPEDIT_ERASER_CURSOR
};

extern struct mapedit mapedit;
extern int mapedition;
extern struct tool *mapedit_tools[];
extern const int mapedit_ntools;

__BEGIN_DECLS
void	mapedit_init(void);
void	mapedit_destroy(void *);
int	mapedit_load(void *, struct netbuf *);
int	mapedit_save(void *, struct netbuf *);

struct window	*objedit_window(void);
void		 objedit_init(void);
void		 objedit_destroy(void);

struct mapview	*tool_mapview(void);
struct window	*tool_window_new(struct tool *, const char *);
void		 tool_bind_key(void *, SDLMod, SDLKey,
		               void (*)(struct mapview *), int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_MAPEDIT_H_ */
