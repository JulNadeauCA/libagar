/*	$Csoft: mapedit.h,v 1.89 2003/09/06 23:05:51 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MAPEDIT_H_
#define _AGAR_MAPEDIT_H_

#include <engine/map.h>

#include "begin_code.h"

struct mapdir;
struct gendir;
struct tool;

enum {
	MAPEDIT_STAMP,
	MAPEDIT_ERASER,
	MAPEDIT_MAGNIFIER,
	MAPEDIT_RESIZE,
	MAPEDIT_PROPEDIT,
	MAPEDIT_SELECT,
	MAPEDIT_SHIFT,
	MAPEDIT_MERGE,
	MAPEDIT_FILL,
	MAPEDIT_FLIP,
	MAPEDIT_POSITION,
	MAPEDIT_NTOOLS
};

struct mapedit_obj {
	struct object	*obj;		/* Object being edited */
	struct window	*win;		/* Generic edition window */
	TAILQ_ENTRY(mapedit_obj) objs;
};

struct mapedit {
	struct object	obj;
	struct tool	*tools[MAPEDIT_NTOOLS];	/* Map edition tools */
	struct tool	*curtool;		/* Selected tool */
	struct map	 copybuf;		/* Copy/paste buffer */
	struct object	 pseudo;		/* Pseudo-object (for deps) */
	TAILQ_HEAD(,mapedit_obj) dobjs;
	TAILQ_HEAD(,mapedit_obj) gobjs;
};

enum {
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
	MAPEDIT_TOOL_POSITION
};

extern struct mapedit	mapedit;
extern int		mapedition;

__BEGIN_DECLS
void	 mapedit_init(void);
int	 mapedit_load(void *, struct netbuf *);
int	 mapedit_save(void *, struct netbuf *);
void	 mapedit_edit_objdata(struct object *);
void	 mapedit_edit_objgen(struct object *);

struct window	*objedit_window(void);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_MAPEDIT_H_ */
