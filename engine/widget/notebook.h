/*	$Csoft: notebook.h,v 1.3 2005/09/27 00:25:23 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_NOTEBOOK_H_
#define _AGAR_WIDGET_NOTEBOOK_H_

#include <engine/widget/widget.h>
#include <engine/widget/box.h>

#include "begin_code.h"

enum ag_notebook_tab_alignment {
	AG_NOTEBOOK_TABS_TOP,
	AG_NOTEBOOK_TABS_BOTTOM,
	AG_NOTEBOOK_TABS_LEFT,
	AG_NOTEBOOK_TABS_RIGHT
};

typedef struct ag_notebook_tab {
	struct ag_box box;
	int label;
	TAILQ_ENTRY(ag_notebook_tab) tabs;
} AG_NotebookTab;

typedef struct ag_notebook {
	struct ag_widget wid;
	enum ag_notebook_tab_alignment tab_align;
	int flags;
#define AG_NOTEBOOK_WFILL 	0x01	/* Expand to fill available width */
#define AG_NOTEBOOK_HFILL 	0x02	/* Expand to fill available height */
#define AG_NOTEBOOK_HIDE_TABS	0x04	/* Don't display the tabs. */
	pthread_mutex_t	lock;
	int bar_w, bar_h;		/* Dimensions of tab button bar */
	int cont_w, cont_h;		/* Dimensions of largest container */
	int tab_rad;			/* Radius for chamfered tab edges */
	struct ag_notebook_tab *sel_tab;
	TAILQ_HEAD(,ag_notebook_tab) tabs;
} AG_Notebook;

__BEGIN_DECLS
AG_Notebook *AG_NotebookNew(void *, int);
void AG_NotebookInit(AG_Notebook *, int);
void AG_NotebookDestroy(void *);
void AG_NotebookDraw(void *);
void AG_NotebookScale(void *, int, int);
void AG_NotebookSetTabAlignment(AG_Notebook *, enum ag_notebook_tab_alignment);

AG_NotebookTab *AG_NotebookAddTab(AG_Notebook *, const char *,
		                  enum ag_box_type);
void AG_NotebookDelTab(AG_Notebook *, AG_NotebookTab *);
void AG_NotebookSelectTab(AG_Notebook *, AG_NotebookTab *);
void AG_NotebookSetTabVisiblity(AG_Notebook *, int);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_NOTEBOOK_H_ */
