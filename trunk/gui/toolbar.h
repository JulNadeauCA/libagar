/*	$Csoft: toolbar.h,v 1.7 2005/09/27 00:25:24 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_TOOLBAR_H_
#define _AGAR_WIDGET_TOOLBAR_H_

#include <agar/gui/widget.h>
#include <agar/gui/box.h>
#include <agar/gui/button.h>

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
	int curRow;
	int flags;
#define AG_TOOLBAR_HOMOGENOUS	0x01	/* Scale buttons homogenously */
#define AG_TOOLBAR_STICKY	0x02	/* Single sticky selection */
} AG_Toolbar;

__BEGIN_DECLS
AG_Toolbar	*AG_ToolbarNew(void *, enum ag_toolbar_type, int, Uint);
void		 AG_ToolbarInit(AG_Toolbar *, enum ag_toolbar_type, int, Uint);
void		 AG_ToolbarScale(void *, int, int);
void	 	 AG_ToolbarDestroy(void *);
__inline__ void	 AG_ToolbarRow(AG_Toolbar *, int);
AG_Button	*AG_ToolbarButton(AG_Toolbar *, const char *, int,
		                  void (*)(AG_Event *), const char *, ...);
AG_Button	*AG_ToolbarButtonIcon(AG_Toolbar *, SDL_Surface *, int,
		                      void (*)(AG_Event *), const char *, ...);
__inline__ void	 AG_ToolbarSeparator(AG_Toolbar *);
void		 AG_ToolbarSelectUnique(AG_Toolbar *, AG_Button *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_TOOLBAR_H_ */
