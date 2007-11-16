/*	Public domain	*/

#ifndef _AGAR_WIDGET_SEPARATOR_H_
#define _AGAR_WIDGET_SEPARATOR_H_

#ifdef _AGAR_INTERNAL
#include <gui/widget.h>
#else
#include <agar/gui/widget.h>
#endif

#include "begin_code.h"

enum ag_separator_type {
	AG_SEPARATOR_HORIZ,
	AG_SEPARATOR_VERT
};

typedef struct ag_separator {
	struct ag_widget wid;
	enum ag_separator_type type;
	Uint padding;				/* Padding in pixels */
	int visible;				/* Visible flag */
} AG_Separator;

__BEGIN_DECLS
extern const AG_WidgetClass agSeparatorClass;

AG_Separator *AG_SeparatorNew(void *, enum ag_separator_type);
AG_Separator *AG_SeparatorNewInv(void *, enum ag_separator_type);
void          AG_SeparatorSetPadding(AG_Separator *, Uint);

#define AG_SeparatorNewHoriz(p) AG_SeparatorNew((p),AG_SEPARATOR_HORIZ)
#define AG_SeparatorNewVert(p) AG_SeparatorNew((p),AG_SEPARATOR_VERT)
#define AG_SeparatorNewHorizInv(p) AG_SeparatorNewInv((p),AG_SEPARATOR_HORIZ)
#define AG_SeparatorNewVertInv(p) AG_SeparatorNewInv((p),AG_SEPARATOR_VERT)
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_SEPARATOR_H_ */
