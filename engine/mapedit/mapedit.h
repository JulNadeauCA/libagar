/*	$Csoft: mapedit.h,v 1.97 2004/03/30 23:44:53 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_MAPEDIT_H_
#define _AGAR_MAPEDIT_H_

#include <engine/mapedit/mapview.h>

#include <engine/widget/window.h>
#include <engine/widget/button.h>

#include "begin_code.h"

struct mapedit {
	struct object obj;
	struct map copybuf;		/* Copy/paste buffer */
	struct object pseudo;		/* Pseudo-object (for depkeeping) */
};

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
	MAPEDIT_ERASER_CURSOR,
	/* Object editor */
	MAPEDIT_CREATE_OBJ,
	MAPEDIT_EDIT_OBJ,
	MAPEDIT_EDITGEN_OBJ,
	MAPEDIT_LOAD_OBJ,
	MAPEDIT_SAVE_OBJ,
	MAPEDIT_DUP_OBJ,
	MAPEDIT_MOVEUP_OBJ,
	MAPEDIT_MOVEDOWN_OBJ,
	MAPEDIT_DESTROY_OBJ
};

extern struct mapedit mapedit;
extern int mapedition;

__BEGIN_DECLS
void		 mapedit_init(void);
void		 mapedit_destroy(void *);
int		 mapedit_load(void *, struct netbuf *);
int		 mapedit_save(void *, struct netbuf *);
struct window	*mapedit_settings(void *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_MAPEDIT_H_ */
