/*	Public domain	*/

#ifndef _AGAR_WIDGET_TOOLBAR_H_
#define _AGAR_WIDGET_TOOLBAR_H_

#ifdef _AGAR_INTERNAL
#include <gui/widget.h>
#include <gui/box.h>
#include <gui/button.h>
#else
#include <agar/gui/widget.h>
#include <agar/gui/box.h>
#include <agar/gui/button.h>
#endif

#include "begin_code.h"

#define AG_TOOLBAR_MAX_ROWS	8

enum ag_toolbar_type {
	AG_TOOLBAR_HORIZ,
	AG_TOOLBAR_VERT
};

typedef struct ag_toolbar {
	struct ag_box box;
	struct ag_box *rows[AG_TOOLBAR_MAX_ROWS];
	enum ag_toolbar_type type;
	int nRows;			/* Number of rows */
	int nButtons;			/* Total number of buttons */
	int curRow;			/* Current row index */
	Uint flags;
#define AG_TOOLBAR_HOMOGENOUS	0x01	/* Scale buttons homogenously */
#define AG_TOOLBAR_STICKY	0x02	/* Single toggle selection */
#define AG_TOOLBAR_MULTI_STICKY	0x04	/* Multiple toggle selections */
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
void		 AG_ToolbarSelect(AG_Toolbar *, AG_Button *);
void		 AG_ToolbarDeselect(AG_Toolbar *, AG_Button *);
void		 AG_ToolbarSelectOnly(AG_Toolbar *, AG_Button *);
void		 AG_ToolbarSelectAll(AG_Toolbar *);
void		 AG_ToolbarDeselectAll(AG_Toolbar *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_TOOLBAR_H_ */
