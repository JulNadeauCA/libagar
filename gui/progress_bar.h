/*	Public domain	*/

#ifndef _AGAR_WIDGET_PROGRESS_BAR_H_
#define _AGAR_WIDGET_PROGRESS_BAR_H_

#ifdef _AGAR_INTERNAL
#include <gui/widget.h>
#else
#include <agar/gui/widget.h>
#endif

#include "begin_code.h"

enum ag_progress_bar_type {
	AG_PROGRESS_BAR_HORIZ,
	AG_PROGRESS_BAR_VERT
};

struct ag_text_cache;

typedef struct ag_progress_bar {
	struct ag_widget wid;
	Uint flags;
#define AG_PROGRESS_BAR_HFILL		0x01
#define AG_PROGRESS_BAR_VFILL		0x02
#define AG_PROGRESS_BAR_SHOW_PCT	0x04	/* Show percent% text */
#define AG_PROGRESS_BAR_EXPAND	(AG_PROGRESS_BAR_HFILL|AG_PROGRESS_BAR_VFILL)
	int value;			/* Default value binding */
	int min, max;			/* Default range binding */
	enum ag_progress_bar_type type;	/* Style */
	int width;			/* Width in pixels */
	int pad;			/* Padding in pixels */
	struct ag_text_cache *tCache;	/* For SHOW_PCT */
} AG_ProgressBar;

__BEGIN_DECLS
extern AG_WidgetClass agProgressBarClass;

AG_ProgressBar *AG_ProgressBarNew(void *, enum ag_progress_bar_type, Uint);
AG_ProgressBar *AG_ProgressBarNewInt(void *, enum ag_progress_bar_type, Uint,
                                     int *, int *, int *);

#define AG_ProgressBarNewHoriz(p,flags) \
	AG_ProgressBarNew((p),AG_PROGRESS_BAR_HORIZ,(flags))
#define AG_ProgressBarNewVert(p,flags) \
	AG_ProgressBarNew((p),AG_PROGRESS_BAR_VERT,(flags))

void	AG_ProgressBarSetWidth(AG_ProgressBar *, int);
int	AG_ProgressBarPercent(AG_ProgressBar *);
__END_DECLS

#include "close_code.h"
#endif	/* _AGAR_WIDGET_PROGRESS_BAR_H_ */
