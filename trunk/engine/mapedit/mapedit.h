/*	$Csoft: mapedit.h,v 1.98 2004/03/31 00:24:09 vedge Exp $	*/
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
	OBJECT_EDITOR_ICON,
	STAMP_TOOL_ICON,
	ERASER_TOOL_ICON,
	NEW_VIEW_ICON,
	MAGNIFIER_TOOL_ICON,
	RESIZE_TOOL_ICON,
	GRID_ICON,
	PROPS_ICON,
	PROPEDIT_ICON,
	EDIT_ICON,
	RIGHT_ARROW_ICON,
	LEFT_ARROW_ICON,
	UP_ARROW_ICON,
	DOWN_ARROW_ICON,
	NODE_EDITOR_ICON,
	LAYER_EDITOR_ICON,
	SELECT_TOOL_ICON,
	SHIFT_TOOL_ICON,
	MERGE_TOOL_ICON,
	FILL_TOOL_ICON,
	FLIP_TOOL_ICON,
	MEDIASEL_ICON,
	POSITION_TOOL_ICON,
	INVERT_TOOL_ICON,
	SETTINGS_ICON,

	SELECT_CURSOR,
	FILL_CURSOR,
	MAGNIFIER_CURSOR,
	VERTRESIZE_CURSOR,
	HORIZRESIZE_CURSOR,
	ERASER_CURSOR,

	OBJCREATE_ICON,
	OBJEDIT_ICON,
	OBJEDIT_GENERIC_ICON,
	OBJLOAD_ICON,
	OBJSAVE_ICON,
	OBJDUP_ICON,
	OBJMOVEUP_ICON,
	OBJMOVEDOWN_ICON,
	TRASH_ICON,

	VGPOINTS_ICON,
	VGLINES_ICON,
	VGTRIANGLES_ICON
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
