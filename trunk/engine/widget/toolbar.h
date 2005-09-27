/*	$Csoft: toolbar.h,v 1.6 2005/09/08 08:18:57 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_TOOLBAR_H_
#define _AGAR_WIDGET_TOOLBAR_H_

#include <engine/widget/widget.h>
#include <engine/widget/box.h>
#include <engine/widget/button.h>

#include "begin_code.h"

#define AG_TOOLBAR_MAX_ROWS	8

enum ag_toolbar_type {
	AG_TOOLBAR_HORIZ,
	AG_TOOLBAR_VERT
};

struct ag_toolbar_button {
	int row;				/* Assigned row */
	AG_Button *bu;
};

typedef struct ag_toolbar {
	struct ag_box box;
	struct ag_box *rows[AG_TOOLBAR_MAX_ROWS];
	struct ag_toolbar_button *buttons;
	enum ag_toolbar_type type;
	int nrows;
	int flags;
#define AG_TOOLBAR_HOMOGENOUS	0x01
} AG_Toolbar;

__BEGIN_DECLS
AG_Toolbar	*AG_ToolbarNew(void *, enum ag_toolbar_type, int, int);
void		 AG_ToolbarInit(AG_Toolbar *, enum ag_toolbar_type, int, int);
void		 AG_ToolbarScale(void *, int, int);
void	 	 AG_ToolbarDestroy(void *);
AG_Button	*AG_ToolbarAddButton(AG_Toolbar *, int, SDL_Surface *, int,
		                     int, void (*)(int, union evarg *),
				     const char *, ...);
void		 AG_ToolbarAddSeparator(AG_Toolbar *, int);
void		 AG_ToolbarSelectUnique(AG_Toolbar *, AG_Button *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_TOOLBAR_H_ */
