/*	Public domain	*/

#ifndef _AGAR_WIDGET_PROGRESS_BAR_H_
#define _AGAR_WIDGET_PROGRESS_BAR_H_

#include <agar/gui/widget.h>
#include <agar/gui/begin.h>

enum ag_progress_bar_type {
	AG_PROGRESS_BAR_HORIZ,
	AG_PROGRESS_BAR_VERT
};

struct ag_text_cache;

typedef struct ag_progress_bar {
	struct ag_widget wid;		/* AG_Widget -> AG_ProgressBar */
	Uint flags;
#define AG_PROGRESS_BAR_HFILL	 0x01
#define AG_PROGRESS_BAR_VFILL	 0x02
#define AG_PROGRESS_BAR_SHOW_PCT 0x04	/* Show percent% text */
#define AG_PROGRESS_BAR_EXCL	 0x08	/* Exclusive binding access */
#define AG_PROGRESS_BAR_EXPAND	 (AG_PROGRESS_BAR_HFILL|AG_PROGRESS_BAR_VFILL)

	int value;			/* Default value binding */
	int min, max;			/* Default range binding */
	enum ag_progress_bar_type type;	/* Style */
	int width;			/* Width in pixels */
	int length;			/* Length in pixels */
	Uint32 _pad;
	struct ag_text_cache *_Nonnull tCache; /* For SHOW_PCT */
} AG_ProgressBar;

#define AGPROGRESSBAR(obj)            ((AG_ProgressBar *)(obj))
#define AGCPROGRESSBAR(obj)           ((const AG_ProgressBar *)(obj))
#define AG_PROGRESSBAR_SELF()          AGPROGRESSBAR( AG_OBJECT(0,"AG_Widget:AG_ProgressBar:*") )
#define AG_PROGRESSBAR_PTR(n)          AGPROGRESSBAR( AG_OBJECT((n),"AG_Widget:AG_ProgressBar:*") )
#define AG_PROGRESSBAR_NAMED(n)        AGPROGRESSBAR( AG_OBJECT_NAMED((n),"AG_Widget:AG_ProgressBar:*") )
#define AG_CONST_PROGRESSBAR_SELF()   AGCPROGRESSBAR( AG_CONST_OBJECT(0,"AG_Widget:AG_ProgressBar:*") )
#define AG_CONST_PROGRESSBAR_PTR(n)   AGCPROGRESSBAR( AG_CONST_OBJECT((n),"AG_Widget:AG_ProgressBar:*") )
#define AG_CONST_PROGRESSBAR_NAMED(n) AGCPROGRESSBAR( AG_CONST_OBJECT_NAMED((n),"AG_Widget:AG_ProgressBar:*") )

__BEGIN_DECLS
extern AG_WidgetClass agProgressBarClass;

AG_ProgressBar *_Nonnull AG_ProgressBarNew(void *_Nullable,
                                           enum ag_progress_bar_type, Uint);
AG_ProgressBar *_Nonnull AG_ProgressBarNewHoriz(void *_Nullable, Uint);
AG_ProgressBar *_Nonnull AG_ProgressBarNewVert(void *_Nullable, Uint);

AG_ProgressBar *_Nonnull AG_ProgressBarNewInt(void *_Nullable,
                                              enum ag_progress_bar_type, Uint,
                                              int *_Nullable,
					      int *_Nullable, int *_Nullable);

void AG_ProgressBarSetLength(AG_ProgressBar *_Nonnull, int);
void AG_ProgressBarSetWidth(AG_ProgressBar *_Nonnull, int);
int  AG_ProgressBarPercent(AG_ProgressBar *_Nonnull);
__END_DECLS

#include <agar/gui/close.h>
#endif	/* _AGAR_WIDGET_PROGRESS_BAR_H_ */
