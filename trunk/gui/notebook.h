/*	Public domain	*/

#ifndef _AGAR_WIDGET_NOTEBOOK_H_
#define _AGAR_WIDGET_NOTEBOOK_H_

#include <agar/gui/widget.h>
#include <agar/gui/box.h>
#include <agar/gui/text.h>
#include <agar/gui/label.h>

#define AG_NOTEBOOK_LABEL_MAX	64

#include <agar/gui/begin.h>

enum ag_notebook_tab_alignment {
	AG_NOTEBOOK_TABS_TOP,
	AG_NOTEBOOK_TABS_BOTTOM,
	AG_NOTEBOOK_TABS_LEFT,
	AG_NOTEBOOK_TABS_RIGHT
};

typedef struct ag_notebook_tab {
	struct ag_box box;
	AG_Label *lbl;				/* Optional text label */
	AG_TAILQ_ENTRY(ag_notebook_tab) tabs;
} AG_NotebookTab;

typedef struct ag_notebook {
	struct ag_widget wid;
	enum ag_notebook_tab_alignment tab_align;
	Uint flags;
#define AG_NOTEBOOK_HFILL 	0x01	/* Expand to fill available width */
#define AG_NOTEBOOK_VFILL 	0x02	/* Expand to fill available height */
#define AG_NOTEBOOK_HIDE_TABS	0x04	/* Don't display the tabs. */
#define AG_NOTEBOOK_EXPAND	(AG_NOTEBOOK_HFILL|AG_NOTEBOOK_VFILL)
	int bar_w, bar_h;		/* Dimensions of tab button bar */
	int cont_w, cont_h;		/* Dimensions of largest container */
	int spacing, padding;		/* Spacing for tabs */
	struct ag_notebook_tab *sel_tab;
	AG_TAILQ_HEAD_(ag_notebook_tab) tabs;
	AG_Rect r;			/* View area */
	Uint nTabs;
} AG_Notebook;

__BEGIN_DECLS
extern AG_WidgetClass agNotebookClass;
extern AG_WidgetClass agNotebookTabClass;

AG_Notebook *AG_NotebookNew(void *, Uint);
void AG_NotebookSetPadding(AG_Notebook *, int);
void AG_NotebookSetSpacing(AG_Notebook *, int);
void AG_NotebookSetTabAlignment(AG_Notebook *, enum ag_notebook_tab_alignment);
void AG_NotebookSetTabVisibility(AG_Notebook *, int);

AG_NotebookTab *AG_NotebookAdd(AG_Notebook *, const char *, enum ag_box_type);
void            AG_NotebookDel(AG_Notebook *, AG_NotebookTab *);
void            AG_NotebookSelect(AG_Notebook *, AG_NotebookTab *);


#ifdef AG_LEGACY
# define AG_NotebookSetTabFont		AG_SetFont
# define AG_NotebookAddTab		AG_NotebookAdd
# define AG_NotebookAddTabVert(nb,lbl)	AG_NotebookAdd((nb),(lbl),AG_BOX_VERT)
# define AG_NotebookAddTabHoriz(nb,lbl)	AG_NotebookAdd((nb),(lbl),AG_BOX_HORIZ)
# define AG_NotebookDelTab		AG_NotebookDel
# define AG_NotebookSelectTab		AG_NotebookSelect
#endif
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_NOTEBOOK_H_ */
