/*	Public domain	*/

#ifndef _AGAR_WIDGET_NOTEBOOK_H_
#define _AGAR_WIDGET_NOTEBOOK_H_

#include <agar/gui/widget.h>
#include <agar/gui/box.h>
#include <agar/gui/text.h>
#include <agar/gui/label.h>

#ifndef AG_NOTEBOOK_LABEL_MAX
#define AG_NOTEBOOK_LABEL_MAX AG_MODEL
#endif

#include <agar/gui/begin.h>

typedef struct ag_notebook_tab {
	struct ag_box box;              /* AG_Box -> AG_NotebookTab */
	AG_Label *_Nullable lbl;        /* Text label */
	int                 id;         /* Numerical ID */
	Uint32 _pad;
	AG_TAILQ_ENTRY(ag_notebook_tab) tabs;
} AG_NotebookTab;

typedef struct ag_notebook {
	struct ag_widget wid;           /* AG_Widget -> AG_Notebook */
	int bar_w, bar_h;               /* Dimensions of tab button bar (or -1) */
	int cont_w, cont_h;             /* Dimensions of largest container (or -1) */
	int mouseOver;                  /* Index of mouseover tab (or -1) */
	int selTabID;                   /* Selected tab ID (or -1) */
	Uint flags;
#define AG_NOTEBOOK_HFILL     0x01      /* Expand to fill available width */
#define AG_NOTEBOOK_VFILL     0x02      /* Expand to fill available height */
#define AG_NOTEBOOK_HIDE_TABS 0x04      /* Don't display the tabs. */
#define AG_NOTEBOOK_EXPAND   (AG_NOTEBOOK_HFILL | AG_NOTEBOOK_VFILL)

	Uint nTabs;                           /* Tab counter */
	AG_NotebookTab *_Nullable selTab;     /* Selected tab */
	AG_Rect r;                            /* Display area */
	AG_TAILQ_HEAD_(ag_notebook_tab) tabs; /* All tabs */
} AG_Notebook;

#define   AGNOTEBOOK(o)        ((AG_Notebook *)(o))
#define  AGcNOTEBOOK(o)        ((const AG_Notebook *)(o))
#define  AG_NOTEBOOK_ISA(o)    (((AGOBJECT(o)->cid & 0xff000000) >> 24) == 0x1A)
#define  AG_NOTEBOOK_SELF()    AGNOTEBOOK(  AG_OBJECT(0,         "AG_Widget:AG_Notebook:*") )
#define  AG_NOTEBOOK_PTR(n)    AGNOTEBOOK(  AG_OBJECT((n),       "AG_Widget:AG_Notebook:*") )
#define  AG_NOTEBOOK_NAMED(n)  AGNOTEBOOK(  AG_OBJECT_NAMED((n), "AG_Widget:AG_Notebook:*") )
#define AG_cNOTEBOOK_SELF()   AGcNOTEBOOK( AG_cOBJECT(0,         "AG_Widget:AG_Notebook:*") )
#define AG_cNOTEBOOK_PTR(n)   AGcNOTEBOOK( AG_cOBJECT((n),       "AG_Widget:AG_Notebook:*") )
#define AG_cNOTEBOOK_NAMED(n) AGcNOTEBOOK( AG_cOBJECT_NAMED((n), "AG_Widget:AG_Notebook:*") )

#define   AGNOTEBOOKTAB(o)        ((AG_NotebookTab *)(o))
#define  AGcNOTEBOOKTAB(o)        ((const AG_NotebookTab *)(o))
#define  AG_NOTEBOOKTAB_ISA(o)    (((AGOBJECT(o)->cid & 0xffff0000) >> 16) == 0x0902)
#define  AG_NOTEBOOKTAB_SELF()    AGNOTEBOOKTAB(  AG_OBJECT(0,         "AG_Widget:AG_Box:AG_NotebookTab:*") )
#define  AG_NOTEBOOKTAB_PTR(n)    AGNOTEBOOKTAB(  AG_OBJECT((n),       "AG_Widget:AG_Box:AG_NotebookTab:*") )
#define  AG_NOTEBOOKTAB_NAMED(n)  AGNOTEBOOKTAB(  AG_OBJECT_NAMED((n), "AG_Widget:AG_Box:AG_NotebookTab:*") )
#define AG_cNOTEBOOKTAB_SELF()   AGcNOTEBOOKTAB( AG_cOBJECT(0,         "AG_Widget:AG_Box:AG_NotebookTab:*") )
#define AG_cNOTEBOOKTAB_PTR(n)   AGcNOTEBOOKTAB( AG_cOBJECT((n),       "AG_Widget:AG_Box:AG_NotebookTab:*") )
#define AG_cNOTEBOOKTAB_NAMED(n) AGcNOTEBOOKTAB( AG_cOBJECT_NAMED((n), "AG_Widget:AG_Box:AG_NotebookTab:*") )

__BEGIN_DECLS
extern AG_WidgetClass agNotebookClass;
extern AG_WidgetClass agNotebookTabClass;

AG_Notebook *_Nonnull AG_NotebookNew(void *_Nullable, Uint);

void AG_NotebookSetPadding(AG_Notebook *_Nonnull, int);
void AG_NotebookSetSpacing(AG_Notebook *_Nonnull, int);
void AG_NotebookSetTabVisibility(AG_Notebook *_Nonnull, int);

AG_NotebookTab *_Nonnull AG_NotebookAdd(AG_Notebook *_Nonnull,
                                        const char *_Nonnull, enum ag_box_type);
AG_NotebookTab *_Nonnull AG_NotebookGetByName(AG_Notebook *_Nonnull, const char *_Nonnull)
                                             _Pure_Attribute;
AG_NotebookTab *_Nonnull AG_NotebookGetByID(AG_Notebook *_Nonnull, int)
                                           _Pure_Attribute;
void                     AG_NotebookSelectByID(AG_Notebook *_Nonnull, int);
void                     AG_NotebookSelect(AG_Notebook *_Nonnull, AG_NotebookTab *_Nullable);
void                     AG_NotebookDel(AG_Notebook *_Nonnull, AG_NotebookTab *_Nonnull);
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
