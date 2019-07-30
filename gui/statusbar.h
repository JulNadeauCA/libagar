/*	Public domain	*/

#ifndef _AGAR_WIDGET_STATUSBAR_H_
#define _AGAR_WIDGET_STATUSBAR_H_

#include <agar/gui/widget.h>
#include <agar/gui/box.h>
#include <agar/gui/label.h>
#include <agar/gui/begin.h>

#ifndef AG_STATUSBAR_MAX_LABELS
#define AG_STATUSBAR_MAX_LABELS (AG_MODEL >> 3)
#endif

typedef struct ag_statusbar {
	struct ag_box box;

	Uint flags;
#define AG_STATUSBAR_HFILL  0x01
#define AG_STATUSBAR_VFILL  0x02
#define AG_STATUSBAR_EXPAND (AG_STATUSBAR_HFILL|AG_STATUSBAR_VFILL)

	Uint              nLabels;
	AG_Label *_Nonnull labels[AG_STATUSBAR_MAX_LABELS];
} AG_Statusbar;

__BEGIN_DECLS
extern AG_WidgetClass agStatusbarClass;

AG_Statusbar *_Nonnull AG_StatusbarNew(void *_Nullable, Uint);
AG_Label     *_Nonnull AG_StatusbarAddLabel(AG_Statusbar *_Nonnull,
                                            const char *_Nonnull, ...);
__END_DECLS

#include <agar/gui/close.h>
#endif /* _AGAR_WIDGET_STATUSBAR_H_ */
