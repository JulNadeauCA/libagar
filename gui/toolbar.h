/*	Public domain	*/

#ifndef _AGAR_WIDGET_TOOLBAR_H_
#define _AGAR_WIDGET_TOOLBAR_H_

#include <agar/gui/widget.h>
#include <agar/gui/box.h>
#include <agar/gui/button.h>
#include <agar/gui/begin.h>

#ifndef AG_TOOLBAR_MAX_ROWS
#define AG_TOOLBAR_MAX_ROWS (AG_MODEL >> 3)
#endif

enum ag_toolbar_type {
	AG_TOOLBAR_HORIZ,
	AG_TOOLBAR_VERT
};

typedef struct ag_toolbar {
	struct ag_box box;		/* AG_Box(3) -> AG_Toolbar */

	struct ag_box *_Nonnull rows[AG_TOOLBAR_MAX_ROWS];

	enum ag_toolbar_type type;	/* Horizontal or vertical */
	int nRows;			/* Number of rows */
	int nButtons;			/* Total number of buttons */
	int curRow;			/* Current row index */
	Uint flags;
#define AG_TOOLBAR_HOMOGENOUS	0x01	/* Scale buttons homogenously */
#define AG_TOOLBAR_STICKY	0x02	/* Single toggle selection */
#define AG_TOOLBAR_MULTI_STICKY	0x04	/* Multiple toggle selections */
#define AG_TOOLBAR_HFILL	0x08
#define AG_TOOLBAR_VFILL	0x10
#define AG_TOOLBAR_EXPAND	(AG_TOOLBAR_HFILL|AG_TOOLBAR_VFILL)
	Uint32 _pad;
} AG_Toolbar;

#define AGTOOLBAR(obj)            ((AG_Toolbar *)(obj))
#define AGCTOOLBAR(obj)           ((const AG_Toolbar *)(obj))
#define AG_TOOLBAR_SELF()          AGTOOLBAR( AG_OBJECT(0,"AG_Widget:AG_Box:AG_Toolbar:*") )
#define AG_TOOLBAR_PTR(n)          AGTOOLBAR( AG_OBJECT((n),"AG_Widget:AG_Box:AG_Toolbar:*") )
#define AG_TOOLBAR_NAMED(n)        AGTOOLBAR( AG_OBJECT_NAMED((n),"AG_Widget:AG_Box:AG_Toolbar:*") )
#define AG_CONST_TOOLBAR_SELF()   AGCTOOLBAR( AG_CONST_OBJECT(0,"AG_Widget:AG_Box:AG_Toolbar:*") )
#define AG_CONST_TOOLBAR_PTR(n)   AGCTOOLBAR( AG_CONST_OBJECT((n),"AG_Widget:AG_Box:AG_Toolbar:*") )
#define AG_CONST_TOOLBAR_NAMED(n) AGCTOOLBAR( AG_CONST_OBJECT_NAMED((n),"AG_Widget:AG_Box:AG_Toolbar:*") )

__BEGIN_DECLS
extern AG_WidgetClass agToolbarClass;

AG_Toolbar *_Nonnull AG_ToolbarNew(void *_Nullable, enum ag_toolbar_type,
                                   int, Uint);

void AG_ToolbarRow(AG_Toolbar *_Nonnull, int);

AG_Button *_Nonnull AG_ToolbarButton(AG_Toolbar *_Nonnull, const char *_Nullable,
                                     int, void (*_Nonnull)(AG_Event *_Nonnull),
			             const char *_Nullable, ...);

AG_Button *_Nonnull AG_ToolbarButtonIcon(AG_Toolbar *_Nonnull,
                                         const AG_Surface *_Nullable, int,
					 void (*_Nonnull)(AG_Event *_Nonnull),
					 const char *_Nullable, ...);

void AG_ToolbarSeparator(AG_Toolbar *_Nonnull);
void AG_ToolbarSelect(AG_Toolbar *_Nonnull, AG_Button *_Nonnull);
void AG_ToolbarDeselect(AG_Toolbar *_Nonnull, AG_Button *_Nonnull);
void AG_ToolbarSelectOnly(AG_Toolbar *_Nonnull, AG_Button *_Nonnull);
void AG_ToolbarSelectAll(AG_Toolbar *_Nonnull);
void AG_ToolbarDeselectAll(AG_Toolbar *_Nonnull);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_TOOLBAR_H_ */
